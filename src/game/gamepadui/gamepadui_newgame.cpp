#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_mode.h"
#include "gamepadui_language.h"
#include "gamepadui_image.h"
#include "gamepadui_frame.h"
#include "gamepadui_genericconfirmation.h"
#include "../shared/scp_save_format.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/TextEntry.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "icommandline.h"
#include "tier1/utlstring.h"
#include "tier0/memdbgon.h"

bool GameHasCommentary()
{
    const char *pszGameDir = CommandLine()->ParmValue( "-game", CommandLine()->ParmValue( "-defaultgamedir", "hl2" ) );
    return !V_strcmp( pszGameDir, "episodic" ) ||
           !V_strcmp( pszGameDir, "ep2" ) ||
           !V_strcmp( pszGameDir, "portal" ) ||
           !V_strcmp( pszGameDir, "lostcoast" );
}

bool GameHasBonusMaps()
{
    const char *pszGameDir = CommandLine()->ParmValue( "-game", CommandLine()->ParmValue( "-defaultgamedir", "hl2" ) );
    return !V_strcmp( pszGameDir, "portal" );
}


namespace
{
const char *const SESSION_SCHEME = GAMEPADUI_RESOURCE_FOLDER "schemesession.res";
const int MAPS_PER_PAGE = 4;

void SessionText( vgui::HFont font, const char *text, int x, int y, int width, Color color, bool center = false )
{
    GamepadUIString localized( text );
    wchar_t line[512]; V_wcsncpy( line, localized.String(), sizeof( line ) );
    int w = 0, h = 0, length = V_wcslen( line );
    vgui::surface()->GetTextSize( font, line, w, h );
    while ( w > width && length > 1 )
    {
        --length; line[length - 1] = 0x2026; line[length] = 0;
        vgui::surface()->GetTextSize( font, line, w, h );
    }
    vgui::surface()->DrawSetTextFont( font );
    vgui::surface()->DrawSetTextColor( color );
    vgui::surface()->DrawSetTextPos( x + ( center ? Max( 0, ( width - w ) / 2 ) : 0 ), y );
    vgui::surface()->DrawPrintText( line, length );
}

class SessionButton : public GamepadUIActionButton
{
    DECLARE_CLASS_SIMPLE( SessionButton, GamepadUIActionButton );
public:
    SessionButton( vgui::Panel *parent, const char *name, const char *text, const char *command )
        : BaseClass( parent, name, text, command ) {}
    void OnKeyCodePressed( vgui::KeyCode code ) OVERRIDE
    {
        if ( code == KEY_ESCAPE || code == KEY_XBUTTON_B ) GetParent()->OnCommand( "action_back" );
        else BaseClass::OnKeyCodePressed( code );
    }
    void OnMouseWheeled( int delta ) OVERRIDE { GetParent()->OnMouseWheeled( delta ); }
};

class SessionEntry : public vgui::TextEntry
{
    DECLARE_CLASS_SIMPLE( SessionEntry, vgui::TextEntry );
public:
    SessionEntry( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
    {
        SetAllowNonAsciiCharacters( true ); SetMultiline( false );
    }
    void ApplySchemeSettings( vgui::IScheme *scheme ) OVERRIDE
    {
        BaseClass::ApplySchemeSettings( scheme ); SetFont( SCPUIGetTextFont( vgui::surface(), scheme, "Session.Body", true ) );
        SetFgColor( Color( 240, 240, 240, 255 ) ); SetBgColor( Color( 24, 24, 24, 255 ) );
        SetSelectionBgColor( Color( 80, 80, 80, 255 ) );
    }
    void OnKeyCodeTyped( vgui::KeyCode code ) OVERRIDE
    {
        if ( code == KEY_ESCAPE ) GetParent()->OnCommand( "action_back" );
        else BaseClass::OnKeyCodeTyped( code );
    }
};

bool SessionContains( const char *text, const char *search )
{
    wchar_t haystack[256], needle[128];
    V_UTF8ToUnicode( text, haystack, sizeof( haystack ) );
    V_UTF8ToUnicode( search, needle, sizeof( needle ) );
    wchar_t *strings[] = { haystack, needle };
    for ( int i = 0; i < ARRAYSIZE( strings ); ++i )
        for ( wchar_t *c = strings[i]; *c; ++c )
        {
            if ( *c >= L'A' && *c <= L'Z' ) *c += L'a' - L'A';
            else if ( *c >= 0x0410 && *c <= 0x042f ) *c += 32;
            else if ( *c == 0x0401 ) *c = 0x0451;
        }
    return wcsstr( haystack, needle ) != NULL;
}

struct SessionMap
{
    char name[64], title[128], image[MAX_PATH], chapterTitle[128];
    int chapter = 0;
};

class SessionMapTile : public SessionButton
{
    DECLARE_CLASS_SIMPLE( SessionMapTile, SessionButton );
public:
    SessionMapTile( vgui::Panel *parent, int slot ) : BaseClass( parent, "SessionMapTile", "", "" )
    {
        char command[32]; V_snprintf( command, sizeof( command ), "map_tile %d", slot ); SetCommand( command );
    }
    void SetMap( const SessionMap &map )
    {
        SetName( map.name ); SetCaption( map.title );
        if ( !V_stricmp( m_ImagePath, map.image ) ) return;
        V_strncpy( m_ImagePath, map.image, sizeof( m_ImagePath ) );
        m_Image.Cleanup();
        if ( !map.image[0] ) return;
        char path[MAX_PATH]; V_snprintf( path, sizeof( path ), "materials/%s.tga", map.image );
        if ( g_pFullFileSystem->FileExists( path, "GAME" ) ) m_Image.SetTGAImage( path );
        else
        {
            V_snprintf( path, sizeof( path ), "materials/%s.vmt", map.image );
            if ( g_pFullFileSystem->FileExists( path, "GAME" ) ) m_Image.SetImage( map.image );
        }
    }
    void Paint() OVERRIDE
    {
        const int captionH = Max( 26, GetTall() / 5 );
        const int imageH = GetTall() - captionH;
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( m_colBackgroundColor );
        vgui::surface()->DrawFilledRect( 0, 0, GetWide(), GetTall() );
        if ( m_Image.IsValid() )
        {
            vgui::surface()->DrawSetTexture( m_Image );
            vgui::surface()->DrawSetColor( Color( 255, 255, 255, 190 + m_colLeftBorder.a() * 65 / 255 ) );
            vgui::surface()->DrawTexturedRect( 1, 1, GetWide() - 1, imageH );
        }
        else
        {
            // A neutral placeholder keeps uncatalogued installed maps usable.
            vgui::surface()->DrawSetTexture( -1 );
            vgui::surface()->DrawSetColor( Color( 51, 51, 51, 255 ) );
            const int step = Max( 18, GetWide() / 9 );
            for ( int x = 0; x < GetWide(); x += step ) vgui::surface()->DrawLine( x, 0, x, imageH );
            for ( int y = 0; y < imageH; y += step ) vgui::surface()->DrawLine( 0, y, GetWide(), y );
        }
        vgui::surface()->DrawSetTexture( -1 );
        vgui::surface()->DrawSetColor( m_colLeftBorder );
        vgui::surface()->DrawOutlinedRect( 0, 0, GetWide(), GetTall() );
        PaintBorders();
        char text[1024]; V_UnicodeToUTF8( GetButtonText().String(), text, sizeof( text ) );
        const vgui::HFont font = m_hTextFont;
        SessionText( font, text, 12, imageH + ( captionH - vgui::surface()->GetFontTall( font ) ) / 2,
            GetWide() - 24, m_colTextColor );
        m_bNavigateTo = false;
    }
private:
    GamepadUIImage m_Image;
    char m_ImagePath[MAX_PATH] = "";
};

class GamepadUISessionPanel;
vgui::DHANDLE<GamepadUISessionPanel> s_SessionPanel;

class GamepadUISessionPanel : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( GamepadUISessionPanel, GamepadUIFrame );
public:
    GamepadUISessionPanel() : BaseClass( GamepadUI::GetInstance().GetBasePanel(), "SessionCreate" )
    {
        SetScheme( vgui::scheme()->LoadSchemeFromFile( SESSION_SCHEME, "SCPSession" ) );
        m_Coop = GamepadUIIsCoopMode(); m_Players = GamepadUIGetModeConfig()->GetInt("default_players",1);
        m_ModeRevision = GamepadUIModeRevision();
        m_Chapters = !m_Coop;
        LoadMaps();
        m_Search = new SessionEntry( this, "SessionMapSearch" ); m_Search->SetMaximumCharCount( 64 );
        m_Search->AddActionSignalTarget( this );
        m_Name = new SessionEntry( this, "SessionName" ); m_Name->SetMaximumCharCount( 63 );
        m_Name->SetText( "SCP: Final Death" );
        m_ChapterTab = new SessionButton( this, "SessionChapters", "#SCPSession_ChaptersTab", "tab_chapters" );
        m_MapTab = new SessionButton( this, "SessionMaps", "#SCPSession_MapsTab", "tab_maps" );
        m_ChapterTab->SetNavRight( m_MapTab ); m_MapTab->SetNavLeft( m_ChapterTab );
        m_Less = new SessionButton( this, "SessionPlayersLess", "-", "players_less" );
        m_More = new SessionButton( this, "SessionPlayersMore", "+", "players_more" );
        m_CheatsButton = new SessionButton( this, "SessionCheats", "", "toggle_cheats" );
        m_Start = new SessionButton( this, "SessionStart", "#SCPSession_Start", "start_session" ); m_Start->SetHighlighted( true );
        m_Back = new SessionButton( this, "SessionBack", "#SCPSession_Back", "action_back" );
        m_Previous = new SessionButton( this, "SessionPrevious", "<", "previous_page" );
        m_Next = new SessionButton( this, "SessionNext", ">", "next_page" );
        for ( int i = 0; i < MAPS_PER_PAGE; ++i ) m_Tiles[i] = new SessionMapTile( this, i );
        m_SelectedImage = new SessionMapTile( this, -1 ); m_SelectedImage->SetName( "SessionSelectedMap" );
        m_SelectedImage->SetMouseInputEnabled( false ); m_SelectedImage->SetKeyBoardInputEnabled( false );
        FilterMaps(); RefreshControls(); Activate(); Layout(); UpdateGradients();
    }
    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        const bool coop = GamepadUIIsCoopMode();
        if ( coop != m_Coop || m_ModeRevision != GamepadUIModeRevision() ) {
            m_ModeRevision = GamepadUIModeRevision(); m_Coop = coop;
            m_Players = GamepadUIGetModeConfig()->GetInt("default_players",1);
            m_Page=0; LoadMaps(); FilterMaps();
        }
        Layout();
    }
    void OnMouseWheeled( int delta ) OVERRIDE { ChangePage( delta > 0 ? -1 : 1 ); }
    MESSAGE_FUNC_PARAMS( OnSearchChanged, "TextChanged", data )
    {
        if ( data->GetPtr( "panel" ) == m_Search ) { m_Page = 0; FilterMaps(); }
    }
    void OnCommand( const char *command ) OVERRIDE
    {
        if ( !V_strcmp( command, "action_back" ) ) { Close(); return; }
        if ( !V_strcmp( command, "tab_chapters" ) || !V_strcmp( command, "tab_maps" ) )
        {
            m_Chapters = !V_strcmp( command, "tab_chapters" );
            m_Page = 0; m_Selected = -1; FilterMaps();
        }
        else if ( !V_strcmp( command, "players_less" ) || !V_strcmp( command, "players_more" ) )
        {
            if ( m_Coop ) m_Players = clamp( m_Players + ( !V_strcmp( command, "players_more" ) ? 1 : -1 ), GamepadUIGetModeConfig()->GetInt("min_players",1), GamepadUIGetModeConfig()->GetInt("max_players",4) );
            RefreshControls();
        }
        else if ( !V_strcmp( command, "toggle_cheats" ) ) { m_Cheats = !m_Cheats; RefreshControls(); }
        else if ( !V_strcmp( command, "previous_page" ) ) ChangePage( -1 );
        else if ( !V_strcmp( command, "next_page" ) ) ChangePage( 1 );
        else if ( !V_strncmp( command, "map_tile ", 9 ) )
        {
            const int tile = atoi( command + 9 ), row = m_Page * MAPS_PER_PAGE + tile;
            if ( tile >= 0 && tile < MAPS_PER_PAGE && row < m_Filtered.Count() )
            {
                m_Selected = m_Filtered[row]; RefreshControls();
            }
        }
        else if ( !V_strcmp( command, "start_session" ) ) StartSession();
        else BaseClass::OnCommand( command );
    }
    void Paint() OVERRIDE
    {
        vgui::surface()->DrawSetTexture( -1 );
        PaintBackgroundGradients();
        vgui::surface()->DrawSetColor( Color( 255, 255, 255, 45 ) );
        vgui::surface()->DrawLine( m_Right - S( 18 ), m_GridY, m_Right - S( 18 ), m_Footer );
        vgui::IScheme *scheme = vgui::scheme()->GetIScheme( GetScheme() );
        const vgui::HFont title = SCPUIGetTextFont( vgui::surface(), scheme, "Session.Title", true );
        const vgui::HFont body = SCPUIGetTextFont( vgui::surface(), scheme, "Session.Body", true ), small = SCPUIGetTextFont( vgui::surface(), scheme, "Session.Small", true );
        const Color white( 245, 245, 245, 255 ), muted( 164, 164, 164, 255 );
        SessionText( title, "#SCPSession_Title", m_Left, m_Y + S( 22 ), m_W - S( 48 ), white );
        SessionText( small, "#SCPSession_Subtitle", m_Left, m_Y + S( 62 ), m_W - S( 48 ), muted );
        SessionText( small, "#SCPSession_Maps", m_Left, m_Y + S( 132 ), S( 125 ), muted );
        SessionText( small, "#SCPSession_Settings", m_Right, m_Y + S( 117 ), m_RightW, muted );
        SessionText( body, GamepadUIGetModeName(), m_Right, m_Y + S( 410 ), m_RightW, white );
        SessionText( small, "#SCPSession_Name", m_Right, m_Y + S( 337 ), m_RightW, muted );
        SessionText( small, "#SCPSession_Players", m_Right, m_Y + S( 462 ), m_RightW - S( 140 ), muted );
        char count[16]; V_snprintf( count, sizeof( count ), "%d", m_Players );
        SessionText( body, count, m_Right + m_RightW - S( 97 ), m_Y + S( 457 ), S( 52 ), white, true );
        SessionText( small, m_Coop ? "#SCPSession_CoopHint" : "#SCPSession_StoryHint", m_Right, m_Y + S( 558 ), m_RightW, muted );
        char pages[64]; V_snprintf( pages, sizeof( pages ), "%d / %d", m_Filtered.IsEmpty() ? 0 : m_Page + 1, Max( 1, ( m_Filtered.Count() + MAPS_PER_PAGE - 1 ) / MAPS_PER_PAGE ) );
        SessionText( small, pages, m_Left + m_GridW / 2 - S( 45 ), m_Footer + S( 12 ), S( 90 ), muted, true );
        if ( m_Filtered.IsEmpty() ) SessionText( body, m_Chapters ? "#SCPSession_NoChapters" : "#SCPSession_NoMaps", m_Left, m_GridY + S( 60 ), m_GridW, muted, true );
    }
private:
    int S( float value ) const { return Max( 1, int( value * m_Scale + 0.5f ) ); }
    void LoadMaps()
    {
        m_Maps.RemoveAll();
        KeyValues *config=GamepadUIGetModeConfig();
        KeyValues *catalog = new KeyValues( "SCPSessionMaps" );
        const char *lists[]={"maps","chapters"};
        for(int kind=0;kind<2;++kind) {
            KeyValues *list=config->FindKey(lists[kind]);
            for(KeyValues *item=list ? list->GetFirstSubKey() : nullptr;item;item=item->GetNextKey()) {
                KeyValues *entry=catalog->FindKey(item->GetString("map"),true);
                const char *fields[]={"title","title_ru","image"};
                for(int field=0;field<3;++field) if(item->FindKey(fields[field]))
                    entry->SetString(kind==1 && field<2 ? (field==0 ? "chapter_title" : "chapter_title_ru") : fields[field],item->GetString(fields[field]));
                if(kind==1) entry->SetInt("chapter",item->GetInt("number",1));
                else entry->SetBool("hidden",item->GetBool("hidden"));
            }
        }
        FileFindHandle_t find;
        for ( const char *file = g_pFullFileSystem->FindFirstEx( "maps/*.bsp", "GAME", &find ); file && m_Maps.Count() < 512; file = g_pFullFileSystem->FindNext( find ) )
        {
            char name[64];
            if ( V_strlen( file ) >= sizeof( name ) + 4 ) continue;
            V_FileBase( file, name, sizeof( name ) );
            if ( !SCPSafeSaveSlot( name ) || !V_strnicmp( name, "background", 10 ) || g_pFullFileSystem->FindIsDirectory( find ) ) continue;
            bool duplicate = false;
            FOR_EACH_VEC( m_Maps, i ) if ( !V_stricmp( name, m_Maps[i].name ) ) duplicate = true;
            if ( duplicate ) continue;
            KeyValues *entry = catalog->FindKey( name ); if ( (entry && entry->GetBool("hidden")) || (!config->GetBool("all_maps",true) && !entry) ) continue;
            SessionMap map = {};
            V_strncpy( map.name, name, sizeof( map.name ) );
            const char *title = entry ? entry->GetString( !V_stricmp( GamepadUIGetLanguageCode(), "RU" ) ? "title_ru" : "title", entry->GetString("title",name) ) : name;
            V_strncpy( map.title, title, sizeof( map.title ) );
            if ( entry )
            {
                map.chapter = clamp( entry->GetInt( "chapter", 0 ), 0, 999 );
                V_strncpy( map.chapterTitle, entry->GetString( !V_stricmp( GamepadUIGetLanguageCode(), "RU" ) ? "chapter_title_ru" : "chapter_title", entry->GetString("chapter_title",title) ), sizeof( map.chapterTitle ) );
            }
            V_snprintf( map.image, sizeof( map.image ), "vgui/maps/%s", name );
            if ( entry ) V_strncpy( map.image, entry->GetString( "image", map.image ), sizeof( map.image ) );
            if ( V_strstr( map.image, ".." ) || V_strstr( map.image, ":" ) || map.image[0] == '/' || map.image[0] == '\\' ) map.image[0] = 0;
            m_Maps.AddToTail( map );
        }
        g_pFullFileSystem->FindClose( find ); catalog->deleteThis();
        m_Maps.Sort( []( const SessionMap *a, const SessionMap *b ) { return V_stricmp( a->name, b->name ); } );
        m_Selected = m_Maps.IsEmpty() ? -1 : 0;
    }
    void FilterMaps()
    {
        char search[128]; m_Search->GetText( search, sizeof( search ) ); m_Filtered.RemoveAll();
        FOR_EACH_VEC( m_Maps, i )
        {
            GamepadUIString title( m_Maps[i].title ); char text[512];
            V_UnicodeToUTF8( title.String(), text, sizeof( text ) );
            if ( m_Chapters && !m_Maps[i].chapter ) continue;
            GamepadUIString chapterTitle( m_Maps[i].chapterTitle ); char chapterText[512];
            V_UnicodeToUTF8( chapterTitle.String(), chapterText, sizeof( chapterText ) );
            if ( search[0] && !SessionContains( m_Maps[i].name, search ) && !SessionContains( text, search ) &&
                 !( m_Chapters && SessionContains( chapterText, search ) ) ) continue;
            if ( m_Chapters )
            {
                int at = 0;
                while ( at < m_Filtered.Count() && m_Maps[m_Filtered[at]].chapter <= m_Maps[i].chapter ) ++at;
                m_Filtered.InsertBefore( at, i );
            }
            else m_Filtered.AddToTail( i );
        }
        if ( m_Page * MAPS_PER_PAGE >= m_Filtered.Count() ) m_Page = 0;
        if ( m_Filtered.Find( m_Selected ) == m_Filtered.InvalidIndex() )
            m_Selected = m_Filtered.IsEmpty() ? -1 : m_Filtered[0];
        RefreshControls();
    }
    void ChangePage( int delta )
    {
        const int pages = Max( 1, ( m_Filtered.Count() + MAPS_PER_PAGE - 1 ) / MAPS_PER_PAGE );
        m_Page = clamp( m_Page + delta, 0, pages - 1 ); RefreshControls();
    }
    void RefreshControls()
    {
        m_ChapterTab->SetHighlighted( m_Chapters ); m_MapTab->SetHighlighted( !m_Chapters );
        m_Less->SetEnabled( m_Coop && m_Players > GamepadUIGetModeConfig()->GetInt("min_players",1) ); m_More->SetEnabled( m_Coop && m_Players < GamepadUIGetModeConfig()->GetInt("max_players",4) );
        m_CheatsButton->SetCaption( m_Cheats ? "#SCPSession_CheatsOn" : "#SCPSession_CheatsOff" );
        m_CheatsButton->SetHighlighted( m_Cheats );
        m_Previous->SetEnabled( m_Page > 0 ); m_Next->SetEnabled( ( m_Page + 1 ) * MAPS_PER_PAGE < m_Filtered.Count() );
        m_Start->SetEnabled( m_Selected >= 0 && GamepadUIGetModeId()[0] ); m_SelectedImage->SetVisible( m_Selected >= 0 );
        if ( m_Selected >= 0 ) { SessionMap display = DisplayMap( m_Selected ); m_SelectedImage->SetMap( display ); m_SelectedImage->SetName( "SessionSelectedMap" ); m_SelectedImage->SetHighlighted( true ); }
        for ( int i = 0; i < MAPS_PER_PAGE; ++i )
        {
            const int row = m_Page * MAPS_PER_PAGE + i; const bool visible = row < m_Filtered.Count();
            m_Tiles[i]->SetVisible( visible );
            if ( visible ) { SessionMap display = DisplayMap( m_Filtered[row] ); m_Tiles[i]->SetMap( display ); m_Tiles[i]->SetHighlighted( m_Filtered[row] == m_Selected ); }
        }
    }
    SessionMap DisplayMap( int index ) const
    {
        SessionMap map = m_Maps[index];
        if ( m_Chapters && map.chapterTitle[0] ) V_strncpy( map.title, map.chapterTitle, sizeof( map.title ) );
        return map;
    }
    void Layout()
    {
        int w, h; GetParent()->GetSize( w, h ); SetBounds( 0, 0, w, h );
        m_Scale = Min( w / 1280.0f, h / 720.0f );
        m_W = Min( w - S( 32 ), S( 1180 ) ); m_H = Min( h - S( 32 ), S( 664 ) );
        m_X = ( w - m_W ) / 2; m_Y = ( h - m_H ) / 2;
        m_Left = m_X + S( 24 ); m_RightW = S( 318 ); m_Right = m_X + m_W - S( 24 ) - m_RightW;
        m_GridW = m_Right - m_Left - S( 36 ); m_GridY = m_Y + S( 176 ); m_Footer = m_Y + m_H - S( 62 );
        m_ChapterTab->SetBounds( m_Left, m_Y + S( 82 ), S( 170 ), S( 35 ) );
        m_MapTab->SetBounds( m_Left + S( 182 ), m_Y + S( 82 ), S( 170 ), S( 35 ) );
        m_Search->SetBounds( m_Left + S( 130 ), m_Y + S( 124 ), Max( S( 80 ), m_GridW - S( 130 ) ), S( 33 ) );
        const int gap = S( 16 ), tileW = ( m_GridW - gap ) / 2, tileH = ( m_Footer - m_GridY - S( 20 ) - gap ) / 2;
        for ( int i = 0; i < MAPS_PER_PAGE; ++i ) m_Tiles[i]->SetBounds( m_Left + ( i % 2 ) * ( tileW + gap ), m_GridY + ( i / 2 ) * ( tileH + gap ), tileW, tileH );
        m_SelectedImage->SetBounds( m_Right, m_Y + S( 144 ), m_RightW, S( 174 ) );
        m_Name->SetBounds( m_Right, m_Y + S( 359 ), m_RightW, S( 36 ) );
        m_Less->SetBounds( m_Right + m_RightW - S( 138 ), m_Y + S( 451 ), S( 34 ), S( 34 ) );
        m_More->SetBounds( m_Right + m_RightW - S( 34 ), m_Y + S( 451 ), S( 34 ), S( 34 ) );
        m_CheatsButton->SetBounds( m_Right, m_Y + S( 508 ), m_RightW, S( 36 ) );
        m_Start->SetBounds( m_Right, m_Footer, m_RightW, S( 40 ) );
        m_Back->SetBounds( m_Left, m_Footer, S( 126 ), S( 40 ) );
        m_Previous->SetBounds( m_Left + m_GridW / 2 - S( 91 ), m_Footer, S( 36 ), S( 36 ) );
        m_Next->SetBounds( m_Left + m_GridW / 2 + S( 55 ), m_Footer, S( 36 ), S( 36 ) );
    }
    void StartSession()
    {
        if ( m_Selected < 0 || m_Selected >= m_Maps.Count() || !GamepadUIGetModeId()[0] ) return;
        char mapFile[128]; V_snprintf( mapFile, sizeof( mapFile ), "maps/%s.bsp", m_Maps[m_Selected].name );
        if ( !g_pFullFileSystem->FileExists( mapFile, "GAME" ) ) { LoadMaps(); FilterMaps(); return; }
        char name[256]; m_Name->GetText( name, sizeof( name ) );
        // The title is data, but must also be safe inside a quoted console value.
        for ( char *c = name; *c; ++c ) if ( (unsigned char)*c < 32 || *c == '"' || *c == ';' || *c == '\\' ) *c = ' ';
        if ( !name[0] ) V_strncpy( name, "SCP: Final Death", sizeof( name ) );
        char command[1024]; V_snprintf( command, sizeof( command ),
            "gamemode %s\ndeathmatch 0\nspmp_coop %d\ncoop %d\nmaxplayers %d\nsv_cheats %d\ncommentary 0\nhostname \"%s\"\nprogress_enable\nmap %s\n",
            GamepadUIGetModeId(), m_Coop, m_Coop, m_Coop ? m_Players : 1, m_Cheats, name, m_Maps[m_Selected].name );
        GamepadUIStartSession( command ); Close();
    }
    CUtlVector<SessionMap> m_Maps;
    CUtlVector<int> m_Filtered;
    SessionMapTile *m_Tiles[MAPS_PER_PAGE], *m_SelectedImage;
    SessionEntry *m_Search, *m_Name;
    SessionButton *m_ChapterTab, *m_MapTab, *m_Less, *m_More, *m_CheatsButton, *m_Start, *m_Back, *m_Previous, *m_Next;
    int m_Selected = -1, m_Page = 0, m_Players = 1, m_ModeRevision = -1;
    bool m_Coop = false, m_Cheats = false, m_Chapters = true;
    float m_Scale = 1;
    int m_X = 0, m_Y = 0, m_W = 0, m_H = 0, m_Left = 0, m_Right = 0, m_RightW = 0, m_GridW = 0, m_GridY = 0, m_Footer = 0;
};
}

CON_COMMAND( gamepadui_opennewgamedialog, "Open the session creation menu." )
{
    if ( s_SessionPanel.Get() ) s_SessionPanel->Activate();
    else s_SessionPanel = new GamepadUISessionPanel();
}
