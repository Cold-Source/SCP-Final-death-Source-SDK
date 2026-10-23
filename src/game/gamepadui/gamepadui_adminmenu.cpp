#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_frame.h"
#include "gamepadui_language.h"
#include "gamepadui_adminmenu.h"
#include "../shared/scp_playermodels.h"
#include "../shared/scp_admin_catalog.h"
#include "matsys_controls/mdlpanel.h"
#include "matsys_controls/matsyscontrols.h"
#include "vgui/ISurface.h"
#include "vgui_controls/TextEntry.h"
#include "tier0/memdbgon.h"

static ConVar cl_scp_playermodel( "cl_scp_playermodel", "models/player.mdl", FCVAR_USERINFO,
    "Session player model from scripts/scp_playermodels.txt." );
static ConVar cl_scp_playerskin( "cl_scp_playerskin", "0", FCVAR_USERINFO,
    "Session player model skin.", true, 0, true, 255 );

bool GamepadUICanOpenAdminMenu()
{
    ConVarRef cheats( "sv_cheats" );
    return GamepadUI::GetInstance().IsInLevel() && cheats.IsValid() && cheats.GetBool();
}

namespace
{
const char *const MODEL_SCHEME = GAMEPADUI_RESOURCE_FOLDER "schememodels.res";
const int PAGE_SIZE = 12;
const int PLAYER_ROWS = 6;
enum AdminTab { TAB_PLAYER, TAB_MODELS, TAB_NPCS, TAB_WEAPONS, TAB_SERVER, TAB_COUNT };

bool MatchesModelName( const char *name, const char *search )
{
    wchar_t haystack[256], needle[256];
    V_UTF8ToUnicode( name, haystack, sizeof( haystack ) );
    V_UTF8ToUnicode( search, needle, sizeof( needle ) );
    wchar_t *strings[] = { haystack, needle };
    for ( int i = 0; i < ARRAYSIZE( strings ); ++i )
        for ( wchar_t *p = strings[i]; *p; ++p )
        {
            if ( *p >= L'A' && *p <= L'Z' ) *p += L'a' - L'A';
            else if ( *p >= 0x0410 && *p <= 0x042f ) *p += 32;
            else if ( *p == 0x0401 ) *p = 0x0451;
        }
    return wcsstr( haystack, needle ) != NULL;
}

class ModelSearch : public vgui::TextEntry
{
    DECLARE_CLASS_SIMPLE( ModelSearch, vgui::TextEntry );
public:
    ModelSearch( vgui::Panel *parent ) : BaseClass( parent, "ModelSearch" )
    {
        SetAllowNonAsciiCharacters( true );
    }
    void ApplySchemeSettings( vgui::IScheme *scheme ) OVERRIDE
    {
        BaseClass::ApplySchemeSettings( scheme );
        SetFont( SCPUIGetTextFont( vgui::surface(), scheme, "Models.Body", true ) );
        SetBgColor( Color( 28, 28, 28, 255 ) );
        SetFgColor( Color( 245, 245, 245, 255 ) );
        SetSelectionBgColor( Color( 90, 90, 90, 255 ) );
    }
};

const char *ModelName( const SCPPlayerModel &model )
{
    return !V_stricmp( GamepadUIGetLanguageCode(), "RU" ) ? model.nameRussian : model.name;
}

const char *CategoryToken( const char *category )
{
    if ( !V_stricmp( category, "security" ) ) return "#SCPModels_Security";
    if ( !V_stricmp( category, "special" ) ) return "#SCPModels_Special";
    return "#SCPModels_Personnel";
}

// Labels remain inside their cards even with long custom model names.
void DrawText( vgui::HFont font, const char *text, int x, int y, int width, Color color, bool centered = false )
{
    GamepadUIString localized( text );
    wchar_t line[512];
    V_wcsncpy( line, localized.String(), sizeof( line ) );
    int w = 0, h = 0;
    vgui::surface()->GetTextSize( font, line, w, h );
    int length = V_wcslen( line );
    if ( w > width && length > 3 )
    {
        do
        {
            --length;
            line[length - 1] = 0x2026;
            line[length] = 0;
            vgui::surface()->GetTextSize( font, line, w, h );
        } while ( w > width && length > 1 );
    }
    vgui::surface()->DrawSetTextFont( font );
    vgui::surface()->DrawSetTextColor( color );
    vgui::surface()->DrawSetTextPos( x + ( centered ? Max( 0, ( width - w ) / 2 ) : 0 ), y );
    vgui::surface()->DrawPrintText( line, V_wcslen( line ) );
    vgui::surface()->DrawFlushText();
}

class ModelPreview : public CMDLPanel
{
    DECLARE_CLASS_SIMPLE( ModelPreview, CMDLPanel );
public:
    ModelPreview( vgui::Panel *parent, bool interactive ) : BaseClass( parent, "ModelPreview" )
    {
        SetMouseInputEnabled( interactive );
        SetKeyBoardInputEnabled( false );
        SetPaintBorderEnabled( false );
        SetGroundGrid( false );
        SetLookAtCamera( false );
        SetIgnoreDoubleClick( true );
        SetCameraFOV( 36 );
        // Draw into each panel's viewport. The engine's shared 3D-paint render
        // target cannot safely cache a grid of previews at different sizes.
        m_bRenderToTexture = false;
    }

    void ApplySchemeSettings( vgui::IScheme *scheme ) OVERRIDE
    {
        BaseClass::ApplySchemeSettings( scheme );
        SetBackgroundColor( 20, 20, 20 );
        SetPaintBorderEnabled( false );
    }

    void LoadModel( const char *path )
    {
        if ( !V_stricmp( m_Path, path ) ) return;
        V_strncpy( m_Path, path, sizeof( m_Path ) );
        MDLCACHE_CRITICAL_SECTION();
        SetMDL( path );
        CStudioHdr *header = m_RootMDL.m_pStudioHdr;
        const char *idleNames[] = { "idle_unarmed", "idle_all_01", "idle_subtle", "idle01", "idle" };
        int sequence = 0;
        bool found = false;
        if ( header && header->IsValid() )
        {
            for ( int name = 0; name < ARRAYSIZE( idleNames ) && !found; ++name )
                for ( int i = 0; i < header->GetNumSeq(); ++i )
                    if ( !V_stricmp( header->pSeqdesc( i ).pszLabel(), idleNames[name] ) )
                    {
                        sequence = i;
                        found = true;
                        break;
                    }
        }
        SetSequence( sequence, true );
        SetModelAnglesAndPosition( QAngle( 0, 0, 0 ), vec3_origin );
        m_RootMDL.m_MDL.m_flPlaybackRate = 1.0f;
        FrameModel();
    }

    int SkinCount()
    {
        MDLCACHE_CRITICAL_SECTION();
        const studiohdr_t *header = m_RootMDL.m_MDL.GetStudioHdr();
        return header ? Max( 1, header->numskinfamilies ) : 1;
    }

    void OnSizeChanged( int w, int h ) OVERRIDE
    {
        BaseClass::OnSizeChanged( w, h );
        if ( m_Path[0] ) FrameModel();
    }

private:
    void FrameModel()
    {
        Vector low, high;
        if ( !GetBoundingBox( low, high ) ) return;
        const Vector center = ( low + high ) * 0.5f;
        const float radius = Max( 12.0f, ( high - low ).Length() * 0.52f );
        SetCameraPositionAndAngles( center + Vector( 140, 0, 0 ), QAngle( 0, 180, 0 ) );
        LookAt( center, radius );
    }
    char m_Path[MAX_PATH] = "";
};

class ModelButton : public GamepadUIActionButton
{
    DECLARE_CLASS_SIMPLE( ModelButton, GamepadUIActionButton );
public:
    ModelButton( vgui::Panel *parent, const char *name, const char *text, const char *command )
        : BaseClass( parent, name, text, command ) {}
    void OnMouseWheeled( int delta ) OVERRIDE { GetParent()->OnMouseWheeled( delta ); }
    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        if ( code == KEY_ESCAPE || code == KEY_XBUTTON_B ) GetParent()->OnCommand( "action_back" );
        else if ( code == KEY_XBUTTON_Y ) GetParent()->OnCommand( "apply" );
        else BaseClass::OnKeyCodePressed( code );
    }
};

class ModelTile : public ModelButton
{
    DECLARE_CLASS_SIMPLE( ModelTile, ModelButton );
public:
    ModelTile( vgui::Panel *parent, int slot ) : BaseClass( parent, "ModelTile", "", "" )
    {
        char command[32];
        V_snprintf( command, sizeof( command ), "tile %d", slot );
        SetCommand( command );
        m_Preview = new ModelPreview( this, false );
    }
    void SetModel( const SCPPlayerModel &model )
    {
        SetCaption( ModelName( model ) );
        m_Preview->LoadModel( model.path );
        m_Preview->SetSkin( 0 );
    }
    void PerformLayout() OVERRIDE
    {
        BaseClass::PerformLayout();
        m_Preview->SetBounds( 2, 2, Max( 1, GetWide() - 4 ), Max( 1, GetTall() - 30 ) );
    }
    void Paint() OVERRIDE
    {
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( m_colBackgroundColor );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( m_colLeftBorder );
        vgui::surface()->DrawOutlinedRect( 0, 0, GetWide(), GetTall() );
        PaintBorders();
        char utf8[1024];
        V_UnicodeToUTF8( GetButtonText().String(), utf8, sizeof( utf8 ) );
        const vgui::HFont font = SCPUIGetTextFont( vgui::surface(), vgui::scheme()->GetIScheme( GetScheme() ), "Button.Small.Font", true );
        DrawText( font, utf8, 7, GetTall() - 25, GetWide() - 14, m_colTextColor, false );
        m_bNavigateTo = false;
    }
    ModelPreview *m_Preview;
};

class AdminMenuPanel : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( AdminMenuPanel, GamepadUIFrame );
public:
    AdminMenuPanel() : BaseClass( GamepadUI::GetInstance().GetBasePanel(), "AdminMenu" )
    {
        SetScheme( vgui::scheme()->LoadSchemeFromFile( MODEL_SCHEME, "SCPPlayerModels" ) );
        UpdateGradients();
        SCPLoadPlayerModels( g_pFullFileSystem, m_Models );
        m_Preview = new ModelPreview( this, true );
        m_Search = new ModelSearch( this );
        m_Search->SetMaximumCharCount( 64 );
        m_Search->AddActionSignalTarget( this );
        m_Back = new ModelButton( this, "ModelBack", "#SCPModels_Back", "action_back" );
        m_Apply = new ModelButton( this, "ModelApply", "#SCPModels_Apply", "apply" );
        m_Close = new ModelButton( this, "AdminClose", "X", "action_back" );
        const char *tabNames[] = { "#SCPAdmin_Player", "#SCPAdmin_Models", "NPC", "#SCPAdmin_Weapons", "#SCPAdmin_Server" };
        for ( int i = 0; i < TAB_COUNT; ++i )
        {
            char command[32]; V_snprintf( command, sizeof( command ), "tab %d", i );
            m_Tabs[i] = new ModelButton( this, "AdminTab", tabNames[i], command );
        }
        const char *selfNames[] = { "#SCPAdmin_Health", "#SCPAdmin_Armor", "#SCPAdmin_Ammo", "#SCPAdmin_God", "#SCPAdmin_Noclip" };
        const char *selfCommands[] = { "self health", "self armor", "self ammo", "self god", "self noclip" };
        for ( int i = 0; i < 5; ++i ) m_Self[i] = new ModelButton( this, "AdminSelfAction", selfNames[i], selfCommands[i] );
        for ( int i = 0; i < PAGE_SIZE; ++i )
        {
            m_SpawnTiles[i] = new ModelTile( this, i );
            char command[32]; V_snprintf( command, sizeof( command ), "spawn %d", i );
            m_SpawnTiles[i]->SetCommand( command );
        }
        const char *settingVars[] = { "sv_gravity", "hl2_walkspeed", "hl2_normspeed", "hl2_sprintspeed", "scp_jump_multiplier" };
        for ( int i = 0; i < 5; ++i )
        {
            m_Settings[i] = new ModelSearch( this );
            m_Settings[i]->SetMaximumCharCount( 10 );
            m_Settings[i]->SetAllowNonAsciiCharacters( false );
            ConVarRef value( settingVars[i] );
            char text[32]; V_snprintf( text, sizeof( text ), "%.2f", value.IsValid() ? value.GetFloat() : 1.0f );
            m_Settings[i]->SetText( text );
            char command[32]; V_snprintf( command, sizeof( command ), "setting %d", i );
            m_SettingApply[i] = new ModelButton( this, "AdminSettingApply", "OK", command );
        }
        m_Gravity = new ModelButton( this, "AdminGravity", "#SCPAdmin_GravityToggle", "gravity" );
        m_AllWeapon = new ModelButton( this, "AdminAllWeapon", "", "allweapon_next" );
        m_GiveAll = new ModelButton( this, "AdminGiveAll", "#SCPAdmin_GiveAll", "allweapon_give" );
        const char *playerActions[] = { "#SCPAdmin_Kick", "#SCPAdmin_Kill", "#SCPAdmin_Bring", "#SCPAdmin_Goto" };
        for ( int row = 0; row < PLAYER_ROWS; ++row )
            for ( int action = 0; action < 4; ++action )
                m_PlayerActions[row][action] = new ModelButton( this, "AdminPlayerAction", playerActions[action], "" );
        m_Prev = new ModelButton( this, "ModelPreviousPage", "<", "previous" );
        m_Next = new ModelButton( this, "ModelNextPage", ">", "next" );
        m_SkinPrev = new ModelButton( this, "ModelPreviousSkin", "<", "skin_previous" );
        m_SkinNext = new ModelButton( this, "ModelNextSkin", ">", "skin_next" );
        const char *categories[] = { "#SCPModels_All", "#SCPModels_Personnel", "#SCPModels_Security", "#SCPModels_Special" };
        for ( int i = 0; i < 4; ++i )
        {
            char command[32]; V_snprintf( command, sizeof( command ), "category %d", i );
            m_Categories[i] = new ModelButton( this, "ModelCategory", categories[i], command );
        }
        for ( int i = 0; i < PAGE_SIZE; ++i ) m_Tiles[i] = new ModelTile( this, i );
        DoModal();
        LayoutBrowser();
        Refilter( true );
        SetTab( TAB_PLAYER );
        m_Tabs[TAB_PLAYER]->RequestFocus();
    }

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        if ( !GamepadUICanOpenAdminMenu() ) { CloseModal(); return; }
        int parentW, parentH;
        GetParent()->GetSize( parentW, parentH );
        if ( m_LastW != parentW || m_LastH != parentH ) LayoutBrowser();
        if ( m_Tab == TAB_SERVER && Plat_FloatTime() >= m_NextPlayerRefresh ) RefreshPlayers();
        Repaint();
    }

    void Paint() OVERRIDE
    {
        vgui::surface()->DrawSetTexture( -1 );
        PaintBackgroundGradients();
        vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
        const vgui::HFont body = SCPUIGetTextFont( vgui::surface(), scheme, "Models.Body", true );
        const vgui::HFont small = SCPUIGetTextFont( vgui::surface(), scheme, "Models.Small", true );
        const Color white( 245, 245, 245, 255 ), muted( 155, 155, 155, 255 );
        DrawText( SCPUIGetTextFont( vgui::surface(), scheme, "Models.Title", true ), "#SCPAdmin_Title", m_X + S( 24 ), m_Y + S( 18 ), m_GridW, white );
        DrawText( small, "#SCPAdmin_Subtitle", m_X + S( 25 ), m_Y + S( 58 ), m_GridW, muted );
        if ( m_Tab != TAB_MODELS ) { PaintAdmin( scheme ); return; }
        DrawText( small, "#SCPModels_Search", m_SearchX, m_Y + S( 17 ), S( 210 ), muted );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 35 ) );
        vgui::surface()->DrawLine( m_PreviewX - S( 15 ), m_GridY, m_PreviewX - S( 15 ), m_FooterY - S( 16 ) );
        if ( m_Selected >= 0 )
        {
            DrawText( body, ModelName( m_Models[m_Selected] ), m_PreviewX, m_GridY, m_PreviewW, white, true );
            DrawText( small, CategoryToken( m_Models[m_Selected].category ), m_PreviewX, m_GridY + S( 28 ), m_PreviewW, muted, true );
            DrawText( small, "#SCPModels_Rotate", m_PreviewX, m_FooterY - S( 105 ), m_PreviewW, muted, true );
            DrawText( small, "#SCPModels_Zoom", m_PreviewX, m_FooterY - S( 84 ), m_PreviewW, muted, true );
            char skin[64]; V_snprintf( skin, sizeof( skin ), "%d / %d", m_Skin + 1, m_Skins );
            DrawText( small, "#SCPModels_Skin", m_PreviewX + S( 42 ), m_FooterY - S( 51 ), m_PreviewW / 2, muted );
            DrawText( body, skin, m_PreviewX + m_PreviewW / 2, m_FooterY - S( 53 ), m_PreviewW / 2 - S( 44 ), white, true );
        }
        if ( m_Filtered.IsEmpty() ) DrawText( body, "#SCPModels_Empty", m_X + S( 24 ), m_GridY + S( 40 ), m_GridW, muted, true );
        char page[64]; V_snprintf( page, sizeof( page ), "%d / %d", m_Page + 1, Max( 1, ( m_Filtered.Count() + PAGE_SIZE - 1 ) / PAGE_SIZE ) );
        DrawText( body, page, m_X + S( 24 ), m_FooterY + S( 7 ), m_GridW, white, true );
        if ( m_Applied ) DrawText( small, "#SCPModels_Applied", m_X + S( 180 ), m_FooterY + S( 10 ), m_GridW / 3, white );
    }

    void OnCommand( const char *command ) OVERRIDE
    {
        if ( !V_stricmp( command, "action_back" ) ) { CloseModal(); return; }
        if ( !V_strnicmp( command, "tab ", 4 ) ) { SetTab( clamp( V_atoi( command + 4 ), 0, TAB_COUNT - 1 ) ); return; }
        if ( !V_strnicmp( command, "self ", 5 ) )
        {
            const char *action = command + 5;
            if ( !V_stricmp( action, "noclip" ) ) Send( "noclip" );
            else
            {
                const char *allowed[] = { "health", "armor", "ammo", "god" };
                for ( int i = 0; i < ARRAYSIZE( allowed ); ++i )
                    if ( !V_stricmp( action, allowed[i] ) ) { char cmd[64]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin %s", allowed[i] ); Send( cmd ); }
            }
            return;
        }
        if ( !V_strnicmp( command, "spawn ", 6 ) )
        {
            const int index = V_atoi( command + 6 );
            const int count = m_Tab == TAB_NPCS ? ARRAYSIZE( SCPAdminNPCs ) : ARRAYSIZE( SCPAdminWeapons );
            if ( ( m_Tab == TAB_NPCS || m_Tab == TAB_WEAPONS ) && index >= 0 && index < count )
            {
                char cmd[64]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin %s %d", m_Tab == TAB_NPCS ? "npc" : "weapon", index ); Send( cmd );
            }
            return;
        }
        if ( !V_strnicmp( command, "setting ", 8 ) )
        {
            const int index = V_atoi( command + 8 );
            if ( index >= 0 && index < 5 )
            {
                char text[32]; m_Settings[index]->GetText( text, sizeof( text ) );
                char *end = NULL; const float value = (float)strtod( text, &end );
                if ( end && end != text && !*end && IsFinite( value ) )
                { char cmd[96]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin setting %d %.3f", index, value ); Send( cmd ); }
            }
            return;
        }
        if ( !V_stricmp( command, "gravity" ) )
        {
            ConVarRef gravity( "sv_gravity" );
            float value = m_GravityRestore;
            if ( gravity.IsValid() && gravity.GetFloat() > 0 ) { m_GravityRestore = gravity.GetFloat(); value = 0; }
            char cmd[96], text[32]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin setting 0 %.3f", value );
            V_snprintf( text, sizeof( text ), "%.2f", value );
            Send( cmd ); m_Settings[0]->SetText( text );
            return;
        }
        if ( !V_stricmp( command, "allweapon_next" ) )
        {
            m_AllWeaponIndex = ( m_AllWeaponIndex + 1 ) % ARRAYSIZE( SCPAdminWeapons );
            m_AllWeapon->SetCaption( SCPAdminWeapons[m_AllWeaponIndex].name ); return;
        }
        if ( !V_stricmp( command, "allweapon_give" ) )
        { char cmd[64]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin allweapon %d", m_AllWeaponIndex ); Send( cmd ); return; }
        if ( !V_strnicmp( command, "player ", 7 ) )
        {
            int action = -1, userID = 0;
            if ( sscanf( command + 7, "%d %d", &action, &userID ) == 2 && action >= 0 && action < 4 )
            {
                const char *actions[] = { "kick", "kill", "bring", "goto" };
                char cmd[96]; V_snprintf( cmd, sizeof( cmd ), "cmd scp_admin player %s %d", actions[action], userID ); Send( cmd );
            }
            return;
        }
        if ( !V_stricmp( command, "apply" ) )
        {
            if ( m_Tab == TAB_MODELS && GamepadUICanOpenAdminMenu() && m_Selected >= 0 && m_Selected < m_Models.Count() )
            {
                cl_scp_playermodel.SetValue( m_Models[m_Selected].path );
                cl_scp_playerskin.SetValue( m_Skin );
                GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "cmd scp_applyplayermodel" );
                m_Applied = true;
            }
            return;
        }
        if ( !V_strnicmp( command, "tile ", 5 ) )
        {
            const int slot = V_atoi( command + 5 );
            const int index = m_Page * PAGE_SIZE + slot;
            if ( slot >= 0 && slot < PAGE_SIZE && index < m_Filtered.Count() ) Select( m_Filtered[index] );
            return;
        }
        if ( !V_strnicmp( command, "category ", 9 ) )
        {
            m_Category = clamp( V_atoi( command + 9 ), 0, 3 ); Refilter( false ); return;
        }
        if ( !V_stricmp( command, "previous" ) || !V_stricmp( command, "next" ) )
        {
            if ( m_Tab == TAB_SERVER )
            { m_PlayerPage += !V_stricmp( command, "next" ) ? 1 : -1; RefreshPlayers(); return; }
            if ( m_Tab != TAB_MODELS ) return;
            m_Page = clamp( m_Page + ( !V_stricmp( command, "next" ) ? 1 : -1 ), 0, Max( 0, ( m_Filtered.Count() - 1 ) / PAGE_SIZE ) );
            RefreshTiles(); return;
        }
        if ( ( !V_stricmp( command, "skin_next" ) || !V_stricmp( command, "skin_previous" ) ) && m_Selected >= 0 )
        {
            m_Skin = ( m_Skin + ( !V_stricmp( command, "skin_next" ) ? 1 : m_Skins - 1 ) ) % m_Skins;
            m_Preview->SetSkin( m_Skin ); m_Applied = false; return;
        }
        BaseClass::OnCommand( command );
    }

    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        if ( code == KEY_ESCAPE || code == KEY_XBUTTON_B ) OnCommand( "action_back" );
        else if ( code == KEY_XBUTTON_Y && m_Tab == TAB_MODELS ) OnCommand( "apply" );
        else if ( code == KEY_PAGEDOWN ) OnCommand( "next" );
        else if ( code == KEY_PAGEUP ) OnCommand( "previous" );
        else BaseClass::OnKeyCodePressed( code );
    }
    void OnMouseWheeled( int delta ) OVERRIDE { if ( delta && ( m_Tab == TAB_MODELS || m_Tab == TAB_SERVER ) ) OnCommand( delta > 0 ? "previous" : "next" ); }

private:
    void Send( const char *command )
    {
        if ( !GamepadUICanOpenAdminMenu() ) return;
        GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( command );
        m_LastRequest = Plat_FloatTime();
    }
    void SetTab( int tab )
    {
        m_Tab = tab;
        const bool models = tab == TAB_MODELS, server = tab == TAB_SERVER;
        for ( int i = 0; i < TAB_COUNT; ++i ) m_Tabs[i]->SetHighlighted( i == tab );
        m_Search->SetVisible( models );
        m_Preview->SetVisible( models && m_Selected >= 0 );
        m_Apply->SetVisible( models );
        m_SkinPrev->SetVisible( models && m_Selected >= 0 ); m_SkinNext->SetVisible( models && m_Selected >= 0 );
        for ( int i = 0; i < 4; ++i ) m_Categories[i]->SetVisible( models );
        for ( int i = 0; i < PAGE_SIZE; ++i ) m_Tiles[i]->SetVisible( models && m_Page * PAGE_SIZE + i < m_Filtered.Count() );
        for ( int i = 0; i < 5; ++i )
        {
            m_Self[i]->SetVisible( tab == TAB_PLAYER );
            m_Settings[i]->SetVisible( server ); m_SettingApply[i]->SetVisible( server );
        }
        m_Gravity->SetVisible( server ); m_AllWeapon->SetVisible( server ); m_GiveAll->SetVisible( server );
        m_AllWeapon->SetCaption( SCPAdminWeapons[m_AllWeaponIndex].name );
        const SCPAdminSpawn *catalog = tab == TAB_NPCS ? SCPAdminNPCs : SCPAdminWeapons;
        const int count = tab == TAB_NPCS ? ARRAYSIZE( SCPAdminNPCs ) : ARRAYSIZE( SCPAdminWeapons );
        for ( int i = 0; i < PAGE_SIZE; ++i )
        {
            const bool show = ( tab == TAB_NPCS || tab == TAB_WEAPONS ) && i < count;
            m_SpawnTiles[i]->SetVisible( show );
            if ( !show ) continue;
            SCPPlayerModel model = {};
            V_strncpy( model.path, catalog[i].model, sizeof( model.path ) );
            V_strncpy( model.name, catalog[i].name, sizeof( model.name ) );
            V_strncpy( model.nameRussian, catalog[i].name, sizeof( model.nameRussian ) );
            const bool hasModel = g_pFullFileSystem->FileExists( model.path, "GAME" ) && g_pFullFileSystem->Size( model.path, "GAME" ) > 0;
            m_SpawnTiles[i]->m_Preview->SetVisible( hasModel );
            if ( hasModel ) m_SpawnTiles[i]->SetModel( model );
            else m_SpawnTiles[i]->SetCaption( catalog[i].name );
        }
        m_Prev->SetVisible( models || server ); m_Next->SetVisible( models || server );
        if ( server ) RefreshPlayers();
        else
            for ( int row = 0; row < PLAYER_ROWS; ++row )
                for ( int action = 0; action < 4; ++action ) m_PlayerActions[row][action]->SetVisible( false );
        if ( models ) RefreshTiles();
        LayoutBrowser();
    }
    void RefreshPlayers()
    {
        // Do not rebind an action while its mouse button is held down.
        for ( int row = 0; row < PLAYER_ROWS; ++row )
            for ( int action = 0; action < 4; ++action )
                if ( m_PlayerActions[row][action]->IsSelected() ) return;
        IVEngineClient *engine = GamepadUI::GetInstance().GetEngineClient();
        m_PlayerCount = 0;
        for ( int i = 1; i <= engine->GetMaxClients(); ++i )
        { player_info_t info; if ( engine->GetPlayerInfo( i, &info ) ) ++m_PlayerCount; }
        m_PlayerPage = clamp( m_PlayerPage, 0, Max( 0, ( m_PlayerCount - 1 ) / PLAYER_ROWS ) );
        int index = 0; m_VisiblePlayers = 0;
        for ( int i = 1; i <= engine->GetMaxClients(); ++i )
        {
            player_info_t info;
            if ( !engine->GetPlayerInfo( i, &info ) ) continue;
            if ( index++ < m_PlayerPage * PLAYER_ROWS || m_VisiblePlayers >= PLAYER_ROWS ) continue;
            const int row = m_VisiblePlayers++;
            V_strncpy( m_PlayerNames[row], info.name, sizeof( m_PlayerNames[row] ) );
            for ( int action = 0; action < 4; ++action )
            {
                char command[64]; V_snprintf( command, sizeof( command ), "player %d %d", action, info.userID );
                m_PlayerActions[row][action]->SetCommand( command );
                m_PlayerActions[row][action]->SetEnabled( action == 1 || i != engine->GetLocalPlayer() );
            }
        }
        for ( int row = 0; row < PLAYER_ROWS; ++row )
            for ( int action = 0; action < 4; ++action ) m_PlayerActions[row][action]->SetVisible( m_Tab == TAB_SERVER && row < m_VisiblePlayers );
        if ( m_Tab == TAB_SERVER )
        { m_Prev->SetEnabled( m_PlayerPage > 0 ); m_Next->SetEnabled( ( m_PlayerPage + 1 ) * PLAYER_ROWS < m_PlayerCount ); }
        m_NextPlayerRefresh = Plat_FloatTime() + 1.0;
    }
    void PaintAdmin( vgui::IScheme *scheme )
    {
        const vgui::HFont body = SCPUIGetTextFont( vgui::surface(), scheme, "Models.Body", true ), small = SCPUIGetTextFont( vgui::surface(), scheme, "Models.Small", true );
        const Color white( 245, 245, 245, 255 ), muted( 155, 155, 155, 255 );
        const int left = m_X + S( 24 ), top = m_GridY;
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 35 ) );
        vgui::surface()->DrawLine( left, m_Y + S( 125 ), m_X + m_W - S( 24 ), m_Y + S( 125 ) );
        if ( m_Tab == TAB_PLAYER )
        {
            DrawText( body, "#SCPAdmin_SelfTitle", left, m_Y + S( 140 ), m_W - S( 48 ), white );
            DrawText( small, "#SCPAdmin_SelfHint", left, top + S( 335 ), m_W - S( 48 ), muted );
        }
        else if ( m_Tab == TAB_NPCS || m_Tab == TAB_WEAPONS )
            DrawText( small, m_Tab == TAB_NPCS ? "#SCPAdmin_NPCHint" : "#SCPAdmin_WeaponHint", left, m_Y + S( 140 ), m_W - S( 48 ), muted );
        else if ( m_Tab == TAB_SERVER )
        {
            IVEngineClient *engine = GamepadUI::GetInstance().GetEngineClient();
            char map[256]; V_FileBase( engine->GetLevelName(), map, sizeof( map ) );
            DrawText( body, map, left, m_Y + S( 140 ), m_ServerW, white );
            char players[64]; V_snprintf( players, sizeof( players ), "%d / %d", m_PlayerCount, engine->GetMaxClients() );
            DrawText( body, "#SCPAdmin_Players", left, top, m_ServerW - S( 80 ), white );
            DrawText( small, players, left + m_ServerW - S( 80 ), top + S( 3 ), S( 80 ), muted, true );
            for ( int row = 0; row < m_VisiblePlayers; ++row )
            {
                const int y = top + S( 40 ) + row * m_PlayerRowH;
                vgui::surface()->DrawSetColor( Color( 25, 25, 25, 255 ) );
                vgui::surface()->DrawFilledRect( left, y, left + m_ServerW, y + m_PlayerRowH - S( 8 ) );
                DrawText( body, m_PlayerNames[row], left + S( 12 ), y + S( 10 ), m_ServerW - S( 375 ), white );
            }
            const char *labels[] = { "#SCPAdmin_Gravity", "#SCPAdmin_Walk", "#SCPAdmin_Move", "#SCPAdmin_Run", "#SCPAdmin_Jump" };
            for ( int i = 0; i < 5; ++i ) DrawText( small, labels[i], m_SettingsX, top + S( i * 69 ), m_SettingsW, muted );
            DrawText( small, "#SCPAdmin_AllWeaponHint", m_SettingsX, m_FooterY - S( 122 ), m_SettingsW, muted );
            char page[32]; V_snprintf( page, sizeof( page ), "%d / %d", m_PlayerPage + 1, Max( 1, ( m_PlayerCount + PLAYER_ROWS - 1 ) / PLAYER_ROWS ) );
            DrawText( small, page, left, m_FooterY - S( 35 ), m_ServerW, muted, true );
        }
        DrawText( small, "#SCPAdmin_HostNote", left + S( 165 ), m_FooterY + S( 9 ), m_W - S( 220 ), muted );
        if ( Plat_FloatTime() - m_LastRequest < 2.0 )
            DrawText( small, "#SCPAdmin_Sent", left, m_FooterY - S( 65 ), m_W - S( 48 ), white );
    }
    MESSAGE_FUNC_PARAMS( OnSearchChanged, "TextChanged", data ) { Refilter( false ); }
    int S( int n ) const { return Max( 1, int( n * m_Scale ) ); }
    void Refilter( bool initial )
    {
        char search[256]; m_Search->GetText( search, sizeof( search ) );
        const char *categories[] = { "", "personnel", "security", "special" };
        m_Filtered.RemoveAll();
        FOR_EACH_VEC( m_Models, i )
        {
            if ( m_Category && V_stricmp( categories[m_Category], m_Models[i].category ) ) continue;
            if ( search[0] && !MatchesModelName( ModelName( m_Models[i] ), search ) && !MatchesModelName( m_Models[i].name, search ) ) continue;
            m_Filtered.AddToTail( i );
        }
        m_Page = 0;
        int selected = m_Filtered.IsEmpty() ? -1 : m_Filtered[0];
        if ( initial )
            FOR_EACH_VEC( m_Filtered, i )
                if ( !V_stricmp( m_Models[m_Filtered[i]].path, cl_scp_playermodel.GetString() ) )
                { selected = m_Filtered[i]; m_Page = i / PAGE_SIZE; break; }
        Select( selected );
        for ( int i = 0; i < 4; ++i ) m_Categories[i]->SetHighlighted( i == m_Category );
        RefreshTiles();
    }
    void Select( int selected )
    {
        m_Selected = selected;
        m_Preview->SetVisible( m_Tab == TAB_MODELS && selected >= 0 );
        m_Apply->SetEnabled( selected >= 0 );
        m_Applied = false;
        if ( selected >= 0 )
        {
            m_Preview->LoadModel( m_Models[selected].path );
            m_Skins = m_Preview->SkinCount();
            m_Skin = !V_stricmp( m_Models[selected].path, cl_scp_playermodel.GetString() ) ? clamp( cl_scp_playerskin.GetInt(), 0, m_Skins - 1 ) : 0;
            m_Preview->SetSkin( m_Skin );
        }
        m_SkinPrev->SetVisible( m_Tab == TAB_MODELS && selected >= 0 ); m_SkinNext->SetVisible( m_Tab == TAB_MODELS && selected >= 0 );
        m_SkinPrev->SetEnabled( m_Skins > 1 ); m_SkinNext->SetEnabled( m_Skins > 1 );
        RefreshTiles();
    }
    void RefreshTiles()
    {
        for ( int i = 0; i < PAGE_SIZE; ++i )
        {
            const int index = m_Page * PAGE_SIZE + i;
            m_Tiles[i]->SetVisible( m_Tab == TAB_MODELS && index < m_Filtered.Count() );
            if ( index < m_Filtered.Count() )
            {
                m_Tiles[i]->SetModel( m_Models[m_Filtered[index]] );
                m_Tiles[i]->SetHighlighted( m_Filtered[index] == m_Selected );
            }
        }
        m_Prev->SetEnabled( m_Page > 0 );
        m_Next->SetEnabled( ( m_Page + 1 ) * PAGE_SIZE < m_Filtered.Count() );
    }
    void LayoutBrowser()
    {
        int w, h; GetParent()->GetSize( w, h ); SetBounds( 0, 0, w, h );
        m_LastW = w; m_LastH = h; m_Scale = Max( 0.5f, h / 900.0f );
        m_W = Min( w - S( 48 ), S( 1400 ) ); m_H = Min( h - S( 48 ), S( 840 ) );
        m_X = ( w - m_W ) / 2; m_Y = ( h - m_H ) / 2;
        m_GridW = int( ( m_W - S( 72 ) ) * 0.64f ); m_GridY = m_Y + S( 178 );
        m_PreviewX = m_X + S( 54 ) + m_GridW; m_PreviewW = m_W - m_GridW - S( 78 );
        m_FooterY = m_Y + m_H - S( 65 );
        m_SearchX = m_X + m_GridW - S( 190 );
        m_Search->SetBounds( m_SearchX, m_Y + S( 40 ), S( 214 ), S( 35 ) );
        vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
        m_Search->SetFont( SCPUIGetTextFont( vgui::surface(), scheme, "Models.Body", true ) );
        m_Search->SetBgColor( Color( 28, 28, 28, 255 ) ); m_Search->SetFgColor( Color( 245, 245, 245, 255 ) );
        m_Close->SetBounds( m_X + m_W - S( 57 ), m_Y + S( 18 ), S( 34 ), S( 34 ) );
        const int tabW = Min( S( 155 ), ( m_W - S( 80 ) ) / TAB_COUNT );
        for ( int i = 0; i < TAB_COUNT; ++i ) m_Tabs[i]->SetBounds( m_X + S( 24 ) + i * ( tabW + S( 8 ) ), m_Y + S( 84 ), tabW, S( 35 ) );
        const int categoryW = ( m_GridW - S( 24 ) ) / 4;
        for ( int i = 0; i < 4; ++i ) m_Categories[i]->SetBounds( m_X + S( 24 ) + i * ( categoryW + S( 8 ) ), m_Y + S( 132 ), categoryW, S( 34 ) );
        const int gap = S( 10 ), tileW = ( m_GridW - 3 * gap ) / 4, tileH = ( m_FooterY - m_GridY - S( 18 ) - 2 * gap ) / 3;
        for ( int i = 0; i < PAGE_SIZE; ++i )
            m_Tiles[i]->SetBounds( m_X + S( 24 ) + ( i % 4 ) * ( tileW + gap ), m_GridY + ( i / 4 ) * ( tileH + gap ), tileW, tileH );
        m_Preview->SetBounds( m_PreviewX, m_GridY + S( 56 ), m_PreviewW, Max( S( 80 ), m_FooterY - m_GridY - S( 175 ) ) );
        m_SkinPrev->SetBounds( m_PreviewX, m_FooterY - S( 57 ), S( 33 ), S( 33 ) );
        m_SkinNext->SetBounds( m_PreviewX + m_PreviewW - S( 33 ), m_FooterY - S( 57 ), S( 33 ), S( 33 ) );
        m_Back->SetBounds( m_X + S( 24 ), m_FooterY, S( 145 ), S( 40 ) );
        m_Apply->SetBounds( m_PreviewX, m_FooterY, m_PreviewW, S( 40 ) );
        m_Apply->SetHighlighted( true );
        m_Prev->SetBounds( m_X + S( 24 ) + m_GridW / 2 - S( 92 ), m_FooterY, S( 36 ), S( 36 ) );
        m_Next->SetBounds( m_X + S( 24 ) + m_GridW / 2 + S( 56 ), m_FooterY, S( 36 ), S( 36 ) );
        LayoutAdmin();
    }
    void LayoutAdmin()
    {
        const int left = m_X + S( 24 ), contentW = m_W - S( 48 );
        const int actionW = ( contentW - S( 32 ) ) / 3;
        for ( int i = 0; i < 5; ++i ) m_Self[i]->SetBounds( left + ( i % 3 ) * ( actionW + S( 16 ) ), m_GridY + ( i / 3 ) * S( 145 ), actionW, S( 120 ) );
        const int gap = S( 12 ), spawnW = ( contentW - 3 * gap ) / 4;
        const int spawnH = ( m_FooterY - m_GridY - S( 20 ) - 2 * gap ) / 3;
        for ( int i = 0; i < PAGE_SIZE; ++i ) m_SpawnTiles[i]->SetBounds( left + ( i % 4 ) * ( spawnW + gap ), m_GridY + ( i / 4 ) * ( spawnH + gap ), spawnW, spawnH );
        m_SettingsW = S( 300 ); m_SettingsX = m_X + m_W - S( 24 ) - m_SettingsW;
        m_ServerW = m_SettingsX - left - S( 24 );
        m_PlayerRowH = Min( S( 65 ), ( m_FooterY - m_GridY - S( 130 ) ) / PLAYER_ROWS );
        for ( int row = 0; row < PLAYER_ROWS; ++row )
            for ( int action = 0; action < 4; ++action )
                m_PlayerActions[row][action]->SetBounds( left + m_ServerW - S( 358 ) + action * S( 88 ), m_GridY + S( 40 ) + row * m_PlayerRowH + S( 4 ), S( 82 ), m_PlayerRowH - S( 16 ) );
        for ( int i = 0; i < 5; ++i )
        {
            m_Settings[i]->SetBounds( m_SettingsX, m_GridY + S( 24 + i * 69 ), m_SettingsW - S( 58 ), S( 33 ) );
            m_SettingApply[i]->SetBounds( m_SettingsX + m_SettingsW - S( 50 ), m_GridY + S( 24 + i * 69 ), S( 50 ), S( 33 ) );
        }
        m_Gravity->SetBounds( m_SettingsX, m_FooterY - S( 168 ), m_SettingsW, S( 34 ) );
        m_AllWeapon->SetBounds( m_SettingsX, m_FooterY - S( 97 ), m_SettingsW, S( 34 ) );
        m_GiveAll->SetBounds( m_SettingsX, m_FooterY - S( 53 ), m_SettingsW, S( 36 ) );
        if ( m_Tab == TAB_SERVER )
        {
            m_Prev->SetBounds( left + m_ServerW / 2 - S( 90 ), m_FooterY - S( 42 ), S( 36 ), S( 32 ) );
            m_Next->SetBounds( left + m_ServerW / 2 + S( 54 ), m_FooterY - S( 42 ), S( 36 ), S( 32 ) );
        }
    }
    CUtlVector<SCPPlayerModel> m_Models;
    CUtlVector<int> m_Filtered;
    ModelPreview *m_Preview;
    ModelTile *m_Tiles[PAGE_SIZE];
    ModelButton *m_Tabs[TAB_COUNT], *m_Categories[4], *m_Back, *m_Apply, *m_Close, *m_Prev, *m_Next, *m_SkinPrev, *m_SkinNext;
    ModelButton *m_Self[5], *m_SettingApply[5], *m_Gravity, *m_AllWeapon, *m_GiveAll, *m_PlayerActions[PLAYER_ROWS][4];
    ModelTile *m_SpawnTiles[PAGE_SIZE];
    ModelSearch *m_Settings[5];
    char m_PlayerNames[PLAYER_ROWS][128] = {};
    int m_Tab = TAB_PLAYER, m_PlayerCount = 0, m_PlayerPage = 0, m_VisiblePlayers = 0, m_AllWeaponIndex = 1;
    int m_ServerW = 0, m_SettingsW = 0, m_SettingsX = 0, m_PlayerRowH = 0;
    double m_NextPlayerRefresh = 0, m_LastRequest = -10;
    float m_GravityRestore = 600;
    vgui::TextEntry *m_Search;
    int m_Selected = -1, m_Page = 0, m_Category = 0, m_Skin = 0, m_Skins = 1;
    int m_X = 0, m_Y = 0, m_W = 0, m_H = 0, m_GridW = 0, m_GridY = 0, m_PreviewX = 0, m_PreviewW = 0, m_FooterY = 0, m_SearchX = 0;
    int m_LastW = 0, m_LastH = 0;
    float m_Scale = 1;
    bool m_Applied = false;
};

vgui::DHANDLE<AdminMenuPanel> s_AdminMenuPanel;
}

void GamepadUIOpenAdminMenu()
{
    if ( !GamepadUICanOpenAdminMenu() ) return;
    if ( !vgui::MaterialSystem() || !vgui::MDLCache() || !vgui::StudioRender() )
    {
        Warning( "SCP: model preview interfaces are unavailable.\n" );
        return;
    }
    GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "gameui_activate" );
    if ( s_AdminMenuPanel.Get() ) s_AdminMenuPanel->Activate();
    else s_AdminMenuPanel = new AdminMenuPanel();
}

CON_COMMAND_F( scp_adminmenu, "Open the admin menu (in-game, sv_cheats 1).", FCVAR_CHEAT )
{
    GamepadUIOpenAdminMenu();
}
