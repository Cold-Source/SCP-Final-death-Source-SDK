#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_frame.h"
#include "gamepadui_button.h"
#include "gamepadui_mode.h"
#include "vgui/ISurface.h"
#include "tier0/memdbgon.h"

namespace
{
const char* const MODE_SCHEME = GAMEPADUI_RESOURCE_FOLDER "schememode.res";

class GamepadUIStoryButton : public GamepadUIButton
{
    DECLARE_CLASS_SIMPLE( GamepadUIStoryButton, GamepadUIButton );
public:
    GamepadUIStoryButton( vgui::Panel* parent )
        : BaseClass( parent, parent, MODE_SCHEME, "story", "#ColdUI_ModeStory", "" ) {}

    void Paint() OVERRIDE
    {
        BaseClass::Paint();
        const int size = Max( 10, GetTall() / 5 );
        const int x = GetWide() - size - GetTall() / 3;
        const int y = ( GetTall() - size ) / 2;
        vgui::surface()->DrawSetColor( m_colTextColor );
        for ( int stroke = 0; stroke < Max( 2, size / 7 ); ++stroke )
        {
            vgui::surface()->DrawLine( x, y + size / 2 + stroke, x + size / 3, y + size + stroke );
            vgui::surface()->DrawLine( x + size / 3, y + size + stroke, x + size, y + stroke );
        }
    }
};

class GamepadUIModePanel : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( GamepadUIModePanel, GamepadUIFrame );
public:
    GamepadUIModePanel()
        : BaseClass( GamepadUI::GetInstance().GetBasePanel(), "ModeSelection" )
    {
        SetScheme( vgui::scheme()->LoadSchemeFromFile( MODE_SCHEME, "SchemeMode" ) );
        SetFooterButtons( FooterButtons::Back );
        m_pStory = new GamepadUIStoryButton( this );
        Activate();
        LayoutCard();
        m_pStory->NavigateTo();
    }

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        LayoutCard();
    }

    void Paint() OVERRIDE
    {
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( Color( 0, 0, 0, 190 ) );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( Color( 18, 18, 18, 250 ) );
        vgui::surface()->DrawFilledRect( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 100 ) );
        vgui::surface()->DrawOutlinedRect( m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight );
        PaintLabel( "#ColdUI_Mode", m_nY + m_nPadding, Color( 255, 255, 255, 255 ) );
        PaintLabel( "#ColdUI_ModeSelected", m_nY + m_nHeight - m_nPadding
            - vgui::surface()->GetFontTall( m_hGenericFont ), Color( 180, 180, 180, 255 ) );
    }

    void OnCommand( const char* command ) OVERRIDE
    {
        // Story is already selected; confirming it only returns to the main menu.
        if ( !V_strcmp( command, "story" ) || !V_strcmp( command, "action_back" ) )
            Close();
        else
            BaseClass::OnCommand( command );
    }

private:
    void PaintLabel( const char* token, int y, Color color )
    {
        const GamepadUIString text( token );
        vgui::surface()->DrawSetTextFont( m_hGenericFont );
        vgui::surface()->DrawSetTextColor( color );
        vgui::surface()->DrawSetTextPos( m_nX + m_nPadding, y );
        vgui::surface()->DrawPrintText( text.String(), text.Length() );
    }

    void LayoutCard()
    {
        m_nPadding = Max( 12, vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 18 ) );
        m_nWidth = Min( GetWide() - 2 * m_nPadding,
            vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 360 ) );
        m_nHeight = Min( GetTall() - 2 * m_nPadding,
            vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 180 ) );
        m_nX = ( GetWide() - m_nWidth ) / 2;
        m_nY = ( GetTall() - m_nHeight ) / 2;
        const int width = Max( 1, m_nWidth - 2 * m_nPadding );
        const int height = Max( 1, Min( m_pStory->GetTall(), m_nHeight / 3 ) );
        m_pStory->m_flWidth = float( width );
        m_pStory->m_flHeight = float( height );
        for ( int state = 0; state < ButtonStates::Count; ++state )
        {
            m_pStory->m_flWidthAnimationValue[state] = float( width );
            m_pStory->m_flHeightAnimationValue[state] = float( height );
        }
        m_pStory->SetBounds( m_nX + m_nPadding, m_nY + ( m_nHeight - height ) / 2, width, height );
    }

    GamepadUIStoryButton* m_pStory = nullptr;
    int m_nX = 0, m_nY = 0, m_nWidth = 0, m_nHeight = 0, m_nPadding = 18;
};

vgui::DHANDLE<GamepadUIModePanel> s_ModePanel;
}

void GamepadUIOpenModeDialog()
{
    if ( s_ModePanel.Get() )
        s_ModePanel->Activate();
    else
        s_ModePanel = new GamepadUIModePanel();
}

CON_COMMAND( gamepadui_openmodedialog, "Open the game mode selector." )
{
    GamepadUIOpenModeDialog();
}
