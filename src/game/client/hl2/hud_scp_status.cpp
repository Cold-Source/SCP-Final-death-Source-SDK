#include "cbase.h"
#include "../../shared/scp_ui_font.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_scp_status.h"
#include "c_basehlplayer.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ammodef.h"
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include <vgui_controls/Panel.h>

#include "tier0/memdbgon.h"

static ConVar cl_scp_statushud( "cl_scp_statushud", "1", FCVAR_ARCHIVE, "Draw SCP's translucent vertical status bars." );
static int s_iArmor = 0;

static void LoadSCPHUDStrings()
{
    g_pVGuiLocalize->AddFile( "resource/scp_hud_english.txt", "GAME", true );
    char language[32]; engine->GetUILanguage( language, sizeof( language ) );
    KeyValues *settings = new KeyValues( "ColdUILanguage" );
    if ( settings->LoadFromFile( filesystem, "cfg/cold_ui_language.res", "DEFAULT_WRITE_PATH" ) )
        V_strncpy( language, settings->GetString( "language", language ), sizeof( language ) );
    settings->deleteThis();
    for ( const char *p = language; *p; ++p ) if ( !( ( *p >= 'a' && *p <= 'z' ) || *p == '_' ) ) return;
    char path[128]; V_snprintf( path, sizeof( path ), "resource/scp_hud_%s.txt", language );
    if ( filesystem->FileExists( path, "GAME" ) ) g_pVGuiLocalize->AddFile( path, "GAME", true );
}

static const wchar_t *HUDLabel( const char *token )
{
    const wchar_t *text = g_pVGuiLocalize->Find( token );
    return text ? text : L"";
}

void SCPStatusHudSetArmor( int armor )
{
    s_iArmor = MAX( 0, armor );
}

bool SCPStatusHudReplacesElement( const char *name )
{
    if ( !cl_scp_statushud.GetBool() )
        return false;
    // Keep the original elements alive for their message handlers and animations.
    const char *replaced[] = { "CHudHealth", "CHudBattery", "CHudSuitPower", "CHudAmmo",
        "CHudSecondaryAmmo", "CHUDQuickInfo", "CHudFlashlight", "CHudHistoryResource" };
    for ( int i = 0; i < ARRAYSIZE( replaced ); ++i )
        if ( !Q_stricmp( name, replaced[i] ) )
            return true;
    return false;
}

class CHudSCPStatus : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( CHudSCPStatus, vgui::Panel );
public:
    CHudSCPStatus( const char *name );
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void VidInit() OVERRIDE;
    void LevelInit() OVERRIDE;
    void ApplySchemeSettings( vgui::IScheme *scheme ) OVERRIDE;
    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
private:
    enum Gauge { HEALTH, ENERGY, ARMOR, AMMO, GAUGE_COUNT };
    int S( float pixels ) const { return MAX( 1, (int)( pixels * m_flScale + 0.5f ) ); }
    void UpdateLayout();
    void ReadAmmo( C_BasePlayer *player );
    void Text( vgui::HFont font, const wchar_t *text, int centerX, int y, int alpha );
    void Symbol( Gauge gauge, int x, int y, int alpha );
    void DrawGauge( Gauge gauge, int x, int top, int bottom, const wchar_t *label, const wchar_t *value );
    float m_flScale;
    float m_flDisplayed[GAUGE_COUNT];
    float m_flTarget[GAUGE_COUNT];
    bool m_bSnap;
    bool m_bHasAmmo;
    int m_iHealth, m_iAmmo, m_iReserve, m_iSecondary;
    bool m_bHasReserve;
    CHandle<C_BaseCombatWeapon> m_hWeapon;
    vgui::HFont m_hLabelFont, m_hValueFont;
};

DECLARE_HUDELEMENT( CHudSCPStatus );

CHudSCPStatus::CHudSCPStatus( const char *name ) : CHudElement( name ), BaseClass( NULL, "HudSCPStatus" ),
    m_flScale( 0.0f ), m_bSnap( true ), m_bHasAmmo( false ), m_iHealth( 0 ), m_iAmmo( 0 ),
    m_iReserve( 0 ), m_iSecondary( -1 ), m_bHasReserve( false ), m_hLabelFont( 0 ), m_hValueFont( 0 )
{
    SetParent( g_pClientMode->GetViewport() );
    SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
    SetPaintBackgroundEnabled( false );
    SetPaintBorderEnabled( false );
    SetMouseInputEnabled( false );
    SetKeyBoardInputEnabled( false );
    SetProportional( false );
}

void CHudSCPStatus::Init() { LoadSCPHUDStrings(); s_iArmor = 0; Reset(); }
void CHudSCPStatus::LevelInit() { s_iArmor = 0; Reset(); }
void CHudSCPStatus::VidInit() { m_flScale = 0.0f; Reset(); }
void CHudSCPStatus::Reset()
{
    m_bSnap = true;
    m_hWeapon = NULL;
    for ( int i = 0; i < GAUGE_COUNT; ++i )
        m_flDisplayed[i] = m_flTarget[i] = 0.0f;
}

void CHudSCPStatus::ApplySchemeSettings( vgui::IScheme *scheme )
{
    BaseClass::ApplySchemeSettings( scheme );
    // Scheme fonts are rebuilt by VGUI during video changes. Ad-hoc font
    // handles can be invalidated later in that same reset and lose all glyphs.
    m_hLabelFont = SCPUIGetTextFont( vgui::surface(), scheme, "SCPHUDLabel", true, vgui::ISurface::FONTFLAG_DROPSHADOW );
    m_hValueFont = SCPUIGetTextFont( vgui::surface(), scheme, "SCPHUDValue", true, vgui::ISurface::FONTFLAG_DROPSHADOW );
    SetPaintBackgroundEnabled( false );
    SetPaintBorderEnabled( false );
    m_flScale = 0.0f;
}

bool CHudSCPStatus::ShouldDraw()
{
    return cl_scp_statushud.GetBool() && C_BasePlayer::GetLocalPlayer() &&
        !engine->IsDrawingLoadingImage() && CHudElement::ShouldDraw();
}

void CHudSCPStatus::UpdateLayout()
{
    int width, height;
    vgui::surface()->GetScreenSize( width, height );
    if ( GetWide() != width || GetTall() != height )
        SetBounds( 0, 0, width, height );
    const float scale = MAX( 0.5f, MIN( width / 1280.0f, height / 720.0f ) );
    if ( fabsf( scale - m_flScale ) < 0.001f )
        return;
    m_flScale = scale;
}

void CHudSCPStatus::ReadAmmo( C_BasePlayer *player )
{
    m_bHasAmmo = m_bHasReserve = false;
    m_iAmmo = m_iReserve = 0;
    m_iSecondary = -1;
    m_flTarget[AMMO] = 0.0f;
    IClientVehicle *vehicle = player->GetVehicle();
    C_BaseCombatWeapon *weapon = player->GetActiveWeapon();
    if ( m_hWeapon != weapon )
    {
        m_hWeapon = weapon;
        m_flDisplayed[AMMO] = 0.0f;
    }
    int capacity = 0;
    if ( vehicle && vehicle->GetPrimaryAmmoType() >= 0 )
    {
        m_bHasAmmo = true;
        m_bHasReserve = vehicle->PrimaryAmmoUsesClips();
        m_iAmmo = m_bHasReserve ? vehicle->GetPrimaryAmmoClip() : vehicle->GetPrimaryAmmoCount();
        m_iReserve = vehicle->GetPrimaryAmmoCount();
        capacity = GetAmmoDef()->MaxCarry( vehicle->GetPrimaryAmmoType() );
    }
    else if ( !vehicle && weapon && weapon->UsesPrimaryAmmo() && weapon->GetPrimaryAmmoType() >= 0 )
    {
        m_bHasAmmo = true;
        m_bHasReserve = weapon->UsesClipsForAmmo1();
        m_iReserve = player->GetAmmoCount( weapon->GetPrimaryAmmoType() );
        m_iAmmo = m_bHasReserve ? weapon->Clip1() : m_iReserve;
        capacity = m_bHasReserve ? weapon->GetMaxClip1() : GetAmmoDef()->MaxCarry( weapon->GetPrimaryAmmoType() );
        if ( weapon->UsesSecondaryAmmo() && weapon->GetSecondaryAmmoType() >= 0 )
            m_iSecondary = weapon->UsesClipsForAmmo2() ? weapon->Clip2() : player->GetAmmoCount( weapon->GetSecondaryAmmoType() );
    }
    if ( m_bHasAmmo )
    {
        m_iAmmo = MAX( 0, m_iAmmo );
        m_iReserve = MAX( 0, m_iReserve );
        m_flTarget[AMMO] = clamp( m_iAmmo / (float)MAX( 1, capacity > 0 ? capacity : m_iAmmo ), 0.0f, 1.0f );
    }
}

void CHudSCPStatus::OnThink()
{
    UpdateLayout();
    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if ( !player ) return;
    m_iHealth = MAX( 0, player->GetHealth() );
    m_flTarget[HEALTH] = clamp( m_iHealth / (float)MAX( 100, player->GetMaxHealth() ), 0.0f, 1.0f );
    C_BaseHLPlayer *hlPlayer = dynamic_cast<C_BaseHLPlayer *>( player );
    m_flTarget[ENERGY] = hlPlayer ? clamp( hlPlayer->m_HL2Local.m_flSuitPower / 100.0f, 0.0f, 1.0f ) : 0.0f;
    m_flTarget[ARMOR] = clamp( s_iArmor / 100.0f, 0.0f, 1.0f );
    ReadAmmo( player );
    for ( int i = 0; i < GAUGE_COUNT; ++i )
    {
        if ( m_bSnap ) m_flDisplayed[i] = m_flTarget[i];
        else m_flDisplayed[i] = Approach( m_flTarget[i], m_flDisplayed[i], gpGlobals->frametime * 3.0f );
    }
    m_bSnap = false;
}

void CHudSCPStatus::Text( vgui::HFont font, const wchar_t *text, int centerX, int y, int alpha )
{
    if ( !font ) return;
    int width, height;
    vgui::surface()->GetTextSize( font, text, width, height );
    vgui::surface()->DrawSetTextFont( font );
    vgui::surface()->DrawSetTextColor( 255, 255, 255, alpha );
    vgui::surface()->DrawSetTextPos( centerX - width / 2, y );
    vgui::surface()->DrawPrintText( text, V_wcslen( text ) );
}

void CHudSCPStatus::Symbol( Gauge gauge, int x, int y, int alpha )
{
    vgui::ISurface *surface = vgui::surface();
    surface->DrawSetColor( 255, 255, 255, alpha );
    const int r = S( 7 ), t = S( 2 );
    if ( gauge == HEALTH )
    {
        surface->DrawFilledRect( x - t, y - r, x + t, y + r );
        surface->DrawFilledRect( x - r, y - t, x + r, y + t );
    }
    else if ( gauge == ENERGY )
    {
        int px[] = { x + S( 2 ), x - S( 5 ), x, x - S( 2 ), x + S( 5 ), x };
        int py[] = { y - S( 9 ), y + S( 1 ), y + S( 1 ), y + S( 9 ), y - S( 1 ), y - S( 1 ) };
        surface->DrawPolyLine( px, py, 6 );
        surface->DrawLine( px[5], py[5], px[0], py[0] );
    }
    else if ( gauge == ARMOR )
    {
        int px[] = { x - r, x + r, x + r, x, x - r, x - r };
        int py[] = { y - r, y - r, y + S( 2 ), y + S( 9 ), y + S( 2 ), y - r };
        surface->DrawPolyLine( px, py, 6 );
        surface->DrawLine( x, y - S( 4 ), x, y + S( 4 ) );
    }
    else
    {
        surface->DrawOutlinedRect( x - S( 4 ), y - S( 3 ), x + S( 4 ), y + S( 8 ) );
        surface->DrawLine( x - S( 4 ), y - S( 3 ), x, y - S( 9 ) );
        surface->DrawLine( x, y - S( 9 ), x + S( 3 ), y - S( 3 ) );
        surface->DrawLine( x - S( 4 ), y + S( 4 ), x + S( 3 ), y + S( 4 ) );
    }
}

void CHudSCPStatus::DrawGauge( Gauge gauge, int x, int top, int bottom, const wchar_t *label, const wchar_t *value )
{
    vgui::ISurface *surface = vgui::surface();
    const int half = S( 5 ), inset = S( 2 );
    const int left = x - half, right = x + half;
    const bool available = gauge != AMMO || m_bHasAmmo;
    const bool low = available && m_flTarget[gauge] <= 0.2f && (gauge == HEALTH || gauge == AMMO || gauge == ENERGY);
    const int alpha = low ? 165 + (int)( 55.0f * ( 0.5f + 0.5f * sinf( gpGlobals->curtime * 5.0f ) ) ) : 180;
    surface->DrawSetColor( 0, 0, 0, 75 );
    surface->DrawFilledRect( left - 1, top - 1, right + 1, bottom + 1 );
    surface->DrawSetColor( 255, 255, 255, available ? 60 : 25 );
    surface->DrawOutlinedRect( left, top, right, bottom );
    const int fill = (int)( ( bottom - top - 2 * inset ) * clamp( m_flDisplayed[gauge], 0.0f, 1.0f ) + 0.5f );
    if ( fill > 0 )
    {
        surface->DrawSetColor( 255, 255, 255, alpha );
        surface->DrawFilledRect( left + inset, bottom - inset - fill, right - inset, bottom - inset );
    }
    Symbol( gauge, x, top - S( 19 ), available ? 195 : 70 );
    Text( m_hLabelFont, label, x, top - S( 42 ), available ? 170 : 75 );
    Text( m_hValueFont, value, x, bottom + S( 9 ), available ? 225 : 90 );
}

void CHudSCPStatus::Paint()
{
    if ( !C_BasePlayer::GetLocalPlayer() ) return;
    UpdateLayout();
    const int bottom = GetTall() - S( 80 ), top = bottom - S( 178 );
    const int hpX = S( 38 ), energyX = S( 101 ), armorX = GetWide() - S( 101 ), ammoX = GetWide() - S( 38 );
    wchar_t value[32];
    V_swprintf_safe( value, L"%d", m_iHealth );
    DrawGauge( HEALTH, hpX, top, bottom, HUDLabel( "SCPHUD_Health" ), value );
    V_swprintf_safe( value, L"%d", (int)( m_flTarget[ENERGY] * 100.0f + 0.5f ) );
    DrawGauge( ENERGY, energyX, top, bottom, HUDLabel( "SCPHUD_Energy" ), value );
    V_swprintf_safe( value, L"%d", s_iArmor );
    DrawGauge( ARMOR, armorX, top, bottom, HUDLabel( "SCPHUD_Armor" ), value );
    if ( m_bHasAmmo ) V_swprintf_safe( value, L"%d", m_iAmmo );
    else V_wcscpy_safe( value, L"\u2014" );
    DrawGauge( AMMO, ammoX, top, bottom, HUDLabel( "SCPHUD_Ammo" ), value );
    if ( m_bHasReserve )
    {
        V_swprintf_safe( value, L"/ %d", m_iReserve );
        Text( m_hLabelFont, value, ammoX, bottom + S( 37 ), 155 );
    }
    if ( m_iSecondary >= 0 )
    {
        wchar_t amount[16], secondary[64]; V_swprintf_safe( amount, L"%d", m_iSecondary );
        g_pVGuiLocalize->ConstructString( secondary, sizeof( secondary ), HUDLabel( "SCPHUD_Secondary" ), 1, amount );
        Text( m_hLabelFont, secondary, ammoX, bottom + S( 53 ), 150 );
    }
}
