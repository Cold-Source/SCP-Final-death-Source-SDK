#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_mainmenu.h"
#include "gamepadui_language.h"
#include "gamepadui_frame.h"
#include "gamepadui_button.h"
#include "language.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Panel.h"

#include "tier0/memdbgon.h"

namespace
{
const char* const LANGUAGE_SETTINGS = "cfg/cold_ui_language.res";
const char* const LANGUAGE_SCHEME = GAMEPADUI_RESOURCE_FOLDER "schemelanguage.res";
char s_szLanguage[32] = "english";
char s_szLanguageCode[4] = "EN";

struct LanguageEntry
{
    const char* name;
    const char* label;
    const char* code;
};

// Newer retail translations are not present in the SDK 2013 enum.
const LanguageEntry s_ExtraLanguages[] = {
    { "latam", "Spanish (Latin America)", "ES" },
    { "ukrainian", "Ukrainian", "UK" },
    { "vietnamese", "Vietnamese", "VI" }
};

bool IsSupportedLanguage( const char* name )
{
    FOR_EACH_LANGUAGE( i )
        if ( !V_stricmp( name, GetLanguageShortName( ELanguage( i ) ) ) )
            return true;
    for ( const LanguageEntry& entry : s_ExtraLanguages )
        if ( !V_stricmp( name, entry.name ) )
        {
            char path[128];
            V_snprintf( path, sizeof( path ), "resource/gameui_%s.txt", name );
            return g_pFullFileSystem->FileExists( path, "GAME" );
        }
    return false;
}

void LoadLanguageStrings( const char* name )
{
    const char* const groups[] = { "valve", "gameui", "hl2", "deck", "cold_ui" };
    // Reset each group to English before overlaying the chosen translation, so
    // missing tokens never retain text from the previous language.
    for ( const char* group : groups )
    {
        char path[128];
        V_snprintf( path, sizeof( path ), "resource/%s_english.txt", group );
        g_pVGuiLocalize->AddFile( path, "GAME", true );
        if ( V_stricmp( name, "english" ) )
        {
            V_snprintf( path, sizeof( path ), "resource/%s_%s.txt", group, name );
            if ( g_pFullFileSystem->FileExists( path, "GAME" ) )
                g_pVGuiLocalize->AddFile( path, "GAME", true );
        }
    }
    V_strncpy( s_szLanguage, name, sizeof( s_szLanguage ) );
    const ELanguage language = PchLanguageToELanguage( name, k_Lang_None );
    if ( language != k_Lang_None )
    {
        const char* code = GetLanguageICUName( language );
        s_szLanguageCode[0] = code[0];
        s_szLanguageCode[1] = code[1];
        s_szLanguageCode[2] = '\0';
        V_strupr( s_szLanguageCode );
    }
    else
        for ( const LanguageEntry& entry : s_ExtraLanguages )
            if ( !V_stricmp( name, entry.name ) )
                V_strncpy( s_szLanguageCode, entry.code, sizeof( s_szLanguageCode ) );
}

bool SelectLanguage( const char* name )
{
    if ( !IsSupportedLanguage( name ) )
        return false;
    KeyValues* settings = new KeyValues( "ColdUILanguage" );
    settings->SetString( "language", name );
    g_pFullFileSystem->CreateDirHierarchy( "cfg", "DEFAULT_WRITE_PATH" );
    const bool saved = settings->SaveToFile( g_pFullFileSystem, LANGUAGE_SETTINGS, "DEFAULT_WRITE_PATH" );
    settings->deleteThis();
    if ( !saved )
    {
        Warning( "[GamepadUI] Could not save language selection.\n" );
        return false;
    }
    LoadLanguageStrings( name );
    GamepadUIMainMenu* menu = static_cast<GamepadUIMainMenu*>( GamepadUI::GetInstance().GetMainMenuPanel() );
    if ( menu )
        menu->RefreshMenuLanguage();
    // Captions support changing their language independently of Steam audio.
    char command[128];
    V_snprintf( command, sizeof( command ), "cc_lang %s\nhost_writeconfig\n", name );
    GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( command );
    GamepadUI_Log( "Language changed to %s.\n", name );
    return true;
}

class GamepadUILanguagePanel;
vgui::DHANDLE<GamepadUILanguagePanel> s_LanguagePanel;

class GamepadUILanguagePanel : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( GamepadUILanguagePanel, GamepadUIFrame );
public:
    GamepadUILanguagePanel()
        : BaseClass( GamepadUI::GetInstance().GetBasePanel(), "LanguageSelection" )
    {
        SetScheme( vgui::scheme()->LoadSchemeFromFile( LANGUAGE_SCHEME, "SchemeLanguage" ) );
        SetFooterButtons( FooterButtons::Back );
        FOR_EACH_LANGUAGE( i )
        {
            const ELanguage language = ELanguage( i );
            const char* token = GetLanguageVGUILocalization( language );
            AddLanguage( GetLanguageShortName( language ),
                         g_pVGuiLocalize->Find( token ) ? token : GetLanguageShortName( language ) );
        }
        for ( const LanguageEntry& entry : s_ExtraLanguages )
            if ( IsSupportedLanguage( entry.name ) )
                AddLanguage( entry.name, entry.label );
        Activate();
        LayoutLanguages();
        m_Buttons[m_nSelected]->NavigateTo();
    }

    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        LayoutLanguages();
    }

    void Paint() OVERRIDE
    {
        // An opaque monochrome card keeps the list readable over any background.
        vgui::surface()->DrawSetColor( Color( 0, 0, 0, 220 ) );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        vgui::surface()->DrawSetColor( Color( 18, 18, 18, 255 ) );
        vgui::surface()->DrawFilledRect( m_nCardX, m_nCardY, m_nCardX + m_nCardW, m_nCardY + m_nCardH );
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 75 ) );
        vgui::surface()->DrawOutlinedRect( m_nCardX, m_nCardY, m_nCardX + m_nCardW, m_nCardY + m_nCardH );
        const GamepadUIString title( "#ColdUI_LanguageTitle" );
        vgui::surface()->DrawSetTextFont( m_hGenericFont );
        vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, 255 ) );
        vgui::surface()->DrawSetTextPos( m_nCardX + m_nPadding, m_nCardY + m_nPadding );
        vgui::surface()->DrawPrintText( title.String(), title.Length() );
    }

    void OnCommand( const char* command ) OVERRIDE
    {
        if ( !V_strcmp( command, "action_back" ) )
            Close();
        else if ( StringHasPrefixCaseSensitive( command, "language " ) )
        {
            if ( SelectLanguage( command + 9 ) )
                Close();
        }
    }

    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        if ( code == KEY_ESCAPE )
            Close();
        else
            BaseClass::OnKeyCodePressed( code );
    }

private:
    void AddLanguage( const char* name, const char* label )
    {
        char command[64];
        V_snprintf( command, sizeof( command ), "language %s", name );
        GamepadUIButton* button = new GamepadUIButton( this, this, LANGUAGE_SCHEME, command, label, "" );
        if ( !V_stricmp( name, s_szLanguage ) )
        {
            m_nSelected = m_Buttons.Count();
            wchar_t marked[256];
            V_snwprintf( marked, ARRAYSIZE( marked ), L"* %ls", button->GetButtonText().String() );
            button->GetButtonText().SetText( marked );
        }
        m_Buttons.AddToTail( button );
    }

    void LayoutLanguages()
    {
        if ( m_Buttons.IsEmpty() ) return;
        m_nPadding = Max( 8, vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), 12 ) );
        const int gap = Max( 4, m_nPadding / 2 );
        const int width = m_Buttons[0]->GetWide();
        const int height = m_Buttons[0]->GetTall();
        const int columns = Clamp( ( GetWide() - 4 * m_nPadding + gap ) / Max( 1, width + gap ), 1, 3 );
        const int rows = ( m_Buttons.Count() + columns - 1 ) / columns;
        const int heading = Max( 28, height + gap );
        m_nCardW = columns * ( width + gap ) - gap + 2 * m_nPadding;
        m_nCardH = rows * ( height + gap ) - gap + heading + 2 * m_nPadding;
        m_nCardX = Max( 0, ( GetWide() - m_nCardW ) / 2 );
        m_nCardY = Max( 0, ( GetTall() - m_nCardH ) / 2 );
        for ( int i = 0; i < m_Buttons.Count(); ++i )
        {
            GamepadUIButton* button = m_Buttons[i];
            button->SetPos( m_nCardX + m_nPadding + ( i % columns ) * ( width + gap ),
                            m_nCardY + m_nPadding + heading + ( i / columns ) * ( height + gap ) );
            button->SetNavUp( i >= columns ? m_Buttons[i - columns] : nullptr );
            button->SetNavDown( i + columns < m_Buttons.Count() ? m_Buttons[i + columns] : nullptr );
            button->SetNavLeft( i % columns ? m_Buttons[i - 1] : nullptr );
            button->SetNavRight( i % columns < columns - 1 && i + 1 < m_Buttons.Count() ? m_Buttons[i + 1] : nullptr );
        }
    }

    CUtlVector<GamepadUIButton*> m_Buttons;
    int m_nSelected = 0;
    int m_nCardX = 0, m_nCardY = 0, m_nCardW = 0, m_nCardH = 0, m_nPadding = 12;
};
}

void GamepadUILoadSavedLanguage()
{
    char language[32];
    GamepadUI::GetInstance().GetEngineClient()->GetUILanguage( language, sizeof( language ) );
    KeyValues* settings = new KeyValues( "ColdUILanguage" );
    if ( settings->LoadFromFile( g_pFullFileSystem, LANGUAGE_SETTINGS, "DEFAULT_WRITE_PATH" ) )
        V_strncpy( language, settings->GetString( "language", language ), sizeof( language ) );
    settings->deleteThis();
    LoadLanguageStrings( IsSupportedLanguage( language ) ? language : "english" );
}

const char* GamepadUIGetLanguageCode()
{
    return s_szLanguageCode;
}

void GamepadUIOpenLanguageDialog()
{
    if ( s_LanguagePanel.Get() )
        s_LanguagePanel->Activate();
    else
        s_LanguagePanel = new GamepadUILanguagePanel();
}

CON_COMMAND( gamepadui_openlanguagedialog, "Open the interface language selector." )
{
    GamepadUIOpenLanguageDialog();
}

CON_COMMAND( gamepadui_setlanguage, "Set and save the interface language by its Valve language identifier." )
{
    if ( args.ArgC() == 2 )
        SelectLanguage( args[1] );
}

CON_COMMAND( gamepadui_languagestatus, "Print the active UI language and localization for diagnostics." )
{
    const GamepadUIString text( "#GameUI_GameMenu_NewGame" );
    char utf8[512];
    V_UnicodeToUTF8( text.String(), utf8, sizeof( utf8 ) );
    GamepadUI_Log( "UI language: %s (%s); NewGame: %s.\n", s_szLanguage, s_szLanguageCode, utf8 );
}
