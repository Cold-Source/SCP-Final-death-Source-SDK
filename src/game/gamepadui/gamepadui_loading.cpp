#include "gamepadui_interface.h"
#include "gamepadui_loading.h"
#include "gamepadui_image.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include "mathlib/mathlib.h"
#include <math.h>

#include "tier0/memdbgon.h"

namespace
{
bool IsLoading()
{
    IVEngineClient* engine = GamepadUI::GetInstance().GetEngineClient();
    return engine && engine->IsDrawingLoadingImage();
}

class GamepadUILoadingOverlay : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( GamepadUILoadingOverlay, vgui::Panel );
public:
    explicit GamepadUILoadingOverlay( vgui::VPANEL parent ) : BaseClass( nullptr, "SCPLoadingOverlay" )
    {
        // Use VPANEL interfaces across modules rather than casting a GameUI Panel.
        SetParent( parent );
        // The native dialog retains a small resource-sized rectangle even though
        // its Paint draws the full-screen loading view. A popup avoids that clip.
        MakePopup( false );
        vgui::ipanel()->SetTopmostPopup( GetVPanel(), true );
        SetZPos( 32760 );
        SetMouseInputEnabled( false );
        SetKeyBoardInputEnabled( false );
        SetPaintBackgroundEnabled( false );
        SetPaintBorderEnabled( false );
    }

    void UpdateLayout()
    {
        int width = 0, height = 0;
        vgui::surface()->GetScreenSize( width, height );
        SetBounds( 0, 0, width, height );
        SetVisible( IsLoading() );
        Repaint();
    }

    void Paint() OVERRIDE
    {
        if ( !IsLoading() || GetWide() <= 0 || GetTall() <= 0 )
            return;
        if ( !m_Logo.IsValid() )
            m_Logo.SetImage( "gamepadui/scp_loading_overlay" );

        // Retail x64 GameUI can paint its legacy dialog underneath this popup.
        // Cover its complete rectangle, including the logo and progress widgets.
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( Color( 28, 28, 28, 255 ) );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );

        const float scale = Max( 0.5f, GetTall() / 1080.0f );
        const float halfSize = Min( GetWide() * 0.16f, GetTall() * 0.105f );
        // The loading screen is indeterminate. Its decoration runs on real time,
        // including frames where the game simulation is paused by map loading.
        const float angle = float( fmod( Plat_FloatTime() * 3.0, 1.0 ) ) * 6.28318530718f;
        DrawLogo( GetWide() * 0.5f, GetTall() * 0.48f, halfSize, angle );

        const int markerSize = Max( 14, int( 28 * scale ) );
        const int thickness = Max( 2, int( 4 * scale ) );
        const int margin = Min( GetWide() / 6, Max( 16, int( 44 * scale ) ) );
        const int right = GetWide() - margin;
        const int lineY = GetTall() - Max( markerSize, int( 30 * scale ) );

        // Cover the native footer and draw a single white decorative line.
        // It is not an invented loading percentage.
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( Color( 28, 28, 28, 255 ) );
        vgui::surface()->DrawFilledRect( 0, Max( 0, GetTall() - int( 104 * scale ) ), GetWide(), GetTall() );
        DrawRail( margin, right, lineY, thickness, markerSize, Color( 255, 255, 255, 255 ) );
        vgui::surface()->DrawSetTexture( -1 );
    }

private:
    void DrawLogo( float x, float y, float halfSize, float angle )
    {
        if ( !m_Logo.IsValid() )
            return;
        float sine = 0.0f, cosine = 1.0f;
        SinCos( angle, &sine, &cosine );
        const Vector2D corners[] = {
            Vector2D( -halfSize, -halfSize ), Vector2D( halfSize, -halfSize ),
            Vector2D( halfSize, halfSize ), Vector2D( -halfSize, halfSize )
        };
        const Vector2D uv[] = {
            Vector2D( 0, 0 ), Vector2D( 1, 0 ), Vector2D( 1, 1 ), Vector2D( 0, 1 )
        };
        vgui::Vertex_t vertices[4];
        for ( int i = 0; i < ARRAYSIZE( vertices ); ++i )
            vertices[i].Init( Vector2D( x + corners[i].x * cosine - corners[i].y * sine,
                                       y + corners[i].x * sine + corners[i].y * cosine ), uv[i] );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
        vgui::surface()->DrawSetTexture( m_Logo );
        vgui::surface()->DrawTexturedPolygon( ARRAYSIZE( vertices ), vertices );
        vgui::surface()->DrawSetTexture( -1 );
    }

    void DrawRail( int left, int right, int y, int thickness, int markerSize, Color color )
    {
        const int end = right - markerSize / 2 - 6;
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( color );
        vgui::surface()->DrawFilledRect( left, y - thickness / 2, end, y + ( thickness + 1 ) / 2 );
        DrawLogo( float( right ), float( y ), markerSize * 0.5f, 0.0f );
    }

    GamepadUIImage m_Logo;
};

vgui::VPANEL FindLoadingDialog( vgui::VPANEL panel, int depth = 0 )
{
    if ( !panel || depth > 32 )
        return 0;
    const char* name = vgui::ipanel()->GetName( panel );
    const char* type = vgui::ipanel()->GetClassName( panel );
    // Dialog names differ between the SDK and retail x64 GameUI builds.
    // Never select our own overlay or watcher as its parent.
    if ( name && V_stristr( name, "SCPLoading" ) )
        return 0;
    if ( vgui::ipanel()->IsVisible( panel ) &&
         ( ( name && V_stristr( name, "Loading" ) ) ||
           ( type && V_stristr( type, "LoadingDialog" ) ) ) )
        return panel;
    const int count = vgui::ipanel()->GetChildCount( panel );
    for ( int i = 0; i < count; ++i )
    {
        const vgui::VPANEL found = FindLoadingDialog( vgui::ipanel()->GetChild( panel, i ), depth + 1 );
        if ( found )
            return found;
    }
    return 0;
}

class GamepadUILoadingWatcher : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( GamepadUILoadingWatcher, vgui::Panel );
public:
    GamepadUILoadingWatcher() : BaseClass( nullptr, "SCPLoadingWatcher" )
    {
        // The main-menu override is hidden during loading on retail x64.
        // Keep the watcher attached to the GameUI root instead.
        SetParent( GamepadUI::GetInstance().GetRootVPanel() );
        SetMouseInputEnabled( false );
        SetKeyBoardInputEnabled( false );
        SetPaintEnabled( false );
        SetPaintBackgroundEnabled( false );
        SetPaintBorderEnabled( false );
        vgui::ivgui()->AddTickSignal( GetVPanel(), 0 );
        m_Overlay = new GamepadUILoadingOverlay( GamepadUI::GetInstance().GetRootVPanel() );
        m_Overlay->UpdateLayout();
        // Supply the panel through GameUI's loading interface as well. This
        // makes it part of progress rendering while client frames are suspended.
        GamepadUI::GetInstance().GetGameUI()->SetLoadingBackgroundDialog( m_Overlay->GetVPanel() );
        GamepadUI_Log( "Registered SCP loading background.\n" );
    }

    ~GamepadUILoadingWatcher()
    {
        vgui::ivgui()->RemoveTickSignal( GetVPanel() );
        GamepadUI::GetInstance().GetGameUI()->SetLoadingBackgroundDialog( 0 );
        if ( m_Overlay.Get() )
            delete m_Overlay.Get();
    }

    void OnTick() OVERRIDE
    {
        if ( m_Overlay.Get() )
            m_Overlay->SetVisible( IsLoading() );
        if ( !IsLoading() )
            return;

        vgui::VPANEL loading = FindLoadingDialog( GamepadUI::GetInstance().GetRootVPanel() );
        for ( int i = 0; !loading && i < vgui::surface()->GetPopupCount(); ++i )
            loading = FindLoadingDialog( vgui::surface()->GetPopup( i ) );
        if ( !loading )
            loading = GamepadUI::GetInstance().GetRootVPanel();
        if ( !m_Overlay.Get() )
        {
            m_Overlay = new GamepadUILoadingOverlay( loading );
            GamepadUI::GetInstance().GetGameUI()->SetLoadingBackgroundDialog( m_Overlay->GetVPanel() );
        }
        else if ( m_Overlay->GetVParent() != loading )
            m_Overlay->SetParent( loading );
        m_Overlay->UpdateLayout();
    }

private:
    vgui::DHANDLE<GamepadUILoadingOverlay> m_Overlay;
};

vgui::DHANDLE<GamepadUILoadingWatcher> s_Watcher;
}

void GamepadUILoadingInitialize()
{
    if ( !s_Watcher.Get() )
        s_Watcher = new GamepadUILoadingWatcher();
}

void GamepadUILoadingShutdown()
{
    if ( s_Watcher.Get() )
        delete s_Watcher.Get();
    s_Watcher = nullptr;
}
