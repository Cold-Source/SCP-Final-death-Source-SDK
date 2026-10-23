#include "gamepadui_news.h"
#include "gamepadui_mode.h"
#include "gamepadui_news_http.h"
#include "gamepadui_news_format.h"
#include "gamepadui_interface.h"
#include "gamepadui_language.h"
#include "gamepadui_string.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlstring.h"
#include "vgui/ISurface.h"
#include <thread>
#include <vector>
#include <string>
#include <atomic>

#include "tier0/memdbgon.h"

namespace
{
const char* const NEWS_CACHE = "cfg/scp_news_cache.vdf";
const char* const NEWS_LOCAL = "gamepadui/news.vdf";
struct NewsEntry
{
    CUtlString title, body, date, titleRussian, bodyRussian, titleEnglish, bodyEnglish, image;
};

bool ParseFeed( const std::string& text, CUtlVector<NewsEntry>& entries )
{
    // Never process filesystem directives from a remote document.
    if ( text.empty() || text.size() > GamepadUINewsHTTP::MAX_FEED_BYTES || !GamepadUINewsFormat::Validate( text ) ||
         text.find( '\0' ) != std::string::npos || V_stristr( text.c_str(), "#include" ) ||
         V_stristr( text.c_str(), "#base" ) ) return false;
    KeyValues* root = new KeyValues( "SCPFDNews" );
    root->UsesEscapeSequences( true );
    const char* source = text.c_str();
    if ( text.size() >= 3 && (unsigned char)source[0] == 0xef && (unsigned char)source[1] == 0xbb && (unsigned char)source[2] == 0xbf ) source += 3;
    bool valid = root->LoadFromBuffer( "SCP-FD-News/news.vdf", source ) &&
                 !V_strcmp( root->GetName(), "SCPFDNews" ) && root->GetInt( "version", 0 ) == 1;
    KeyValues* items = root->FindKey( "entries" );
    valid = valid && items != nullptr;
    CUtlVector<NewsEntry> parsed;
    for ( KeyValues* item = valid ? items->GetFirstTrueSubKey() : nullptr; item; item = item->GetNextTrueSubKey() )
    {
        if ( parsed.Count() >= 20 ) { valid = false; break; }
        NewsEntry entry;
        entry.title = item->GetString( "title" ); entry.body = item->GetString( "body" ); entry.date = item->GetString( "date" );
        entry.titleRussian = item->GetString( "title_russian" ); entry.bodyRussian = item->GetString( "body_russian" );
        entry.titleEnglish = item->GetString( "title_english" ); entry.bodyEnglish = item->GetString( "body_english" );
        entry.image = item->GetString( "image" );
        if ( !entry.image.IsEmpty() && !GamepadUINewsHTTP::IsImagePath( entry.image.Get() ) ) valid = false;
        const CUtlString* titles[] = { &entry.title, &entry.titleRussian, &entry.titleEnglish };
        const CUtlString* bodies[] = { &entry.body, &entry.bodyRussian, &entry.bodyEnglish };
        for ( const CUtlString* title : titles ) if ( title->Length() > 512 ) valid = false;
        for ( const CUtlString* body : bodies ) if ( body->Length() > 8192 ) valid = false;
        if ( entry.date.Length() > 32 || (entry.title.IsEmpty() && entry.titleRussian.IsEmpty() && entry.titleEnglish.IsEmpty()) ) valid = false;
        if ( !valid ) break;
        parsed.AddToTail( entry );
    }
    root->deleteThis();
    if ( valid ) entries.Swap( parsed );
    return valid;
}

class NewsService
{
public:
    struct ImageAsset
    {
        std::string path;
        int texture = -1;
        unsigned width = 0, height = 0;
        bool attempted = false;
    };
    ~NewsService() { Shutdown(); }
    void Initialize()
    {
        if ( m_Initialized || !ConfigureMode() ) return;
        m_Initialized = true;
        KeyValues *entries=GamepadUIGetModeConfig()->FindKey("news/entries");
        if(entries) {
            KeyValues *feed=new KeyValues("SCPFDNews"); feed->UsesEscapeSequences(true); feed->SetInt("version",1);
            KeyValues *items=entries->MakeCopy(); items->SetName("entries"); feed->AddSubKey(items);
            CUtlBuffer text(0,0,CUtlBuffer::TEXT_BUFFER); feed->RecursiveSaveToFile(text,0,false,true);
            m_HasFeed=ParseFeed(std::string((const char*)text.Base(),text.TellPut()),m_Entries); feed->deleteThis();
        } else if ( m_Repository.empty() || !LoadFile( m_CacheFile.c_str(), "MOD" ) ) LoadFile( m_LocalFile.c_str(), "GAME" );
        RefreshImages();
    }
    void Update()
    {
        if ( m_Shutdown || !ConfigureMode() ) return;
        Initialize();
        if ( !GamepadUIGetModeConfig()->GetBool("news/enabled",true) ) return;
        if ( m_Worker.joinable() && m_Done.load() )
        {
            m_Worker.join();
            if ( m_ImageJob >= 0 )
            {
                ImageAsset& asset = m_Images[m_ImageJob];
                if ( m_Success )
                {
                    if ( asset.texture <= 0 ) asset.texture = vgui::surface()->CreateNewTextureID( true );
                    vgui::surface()->DrawSetTextureRGBA( asset.texture, m_DecodedImage.rgba.data(), m_DecodedImage.width, m_DecodedImage.height, true, true );
                    asset.width = m_DecodedImage.width; asset.height = m_DecodedImage.height;
                    g_pFullFileSystem->CreateDirHierarchy( "cfg/scp_news_images", "DEFAULT_WRITE_PATH" );
                    CUtlBuffer buffer( 0, 0 ); buffer.Put( m_Response.data(), (int)m_Response.size() );
                    g_pFullFileSystem->WriteFile( ImageCachePath( asset.path ).c_str(), "DEFAULT_WRITE_PATH", buffer );
                    Msg( "SCP news: loaded image %s.\n", asset.path.c_str() );
                }
                asset.attempted = true; m_ImageJob = -1; ++m_Revision;
                m_DecodedImage.rgba.clear();
            }
            else if ( m_Success && ParseFeed( m_Response, m_Entries ) )
            {
                m_HasFeed = true; m_Offline = false; ++m_Revision;
                RefreshImages();
                g_pFullFileSystem->CreateDirHierarchy( "cfg/scp_news", "DEFAULT_WRITE_PATH" );
                CUtlBuffer buffer( 0, 0 ); buffer.Put( m_Response.data(), (int)m_Response.size() );
                g_pFullFileSystem->WriteFile( m_CacheFile.c_str(), "DEFAULT_WRITE_PATH", buffer );
                Msg( "SCP news: loaded %d entries from GitHub.\n", m_Entries.Count() );
                m_NextRequest = Plat_FloatTime() + 300.0;
            }
            else
            {
                m_Offline = true; ++m_Revision;
                DevMsg( "SCP news: update unavailable (HTTP %u); retaining cached news.\n", m_Status );
                m_NextRequest = Plat_FloatTime() + 60.0;
            }
            m_Response.clear();
        }
        if ( !m_Repository.empty() && !m_Worker.joinable() && Plat_FloatTime() >= m_NextRequest )
        {
            m_Done.store( false ); m_Cancel.store( false ); m_Response.clear();
            try
            {
                m_Worker = std::thread( [this]() {
                    try { m_Success = GamepadUINewsHTTP::Download( m_Response, m_Status, m_Cancel, m_Repository, m_Branch, m_FeedFile ); }
                    catch ( ... ) { m_Success = false; }
                    m_Done.store( true );
                } );
            }
            catch ( ... ) { m_Offline = true; ++m_Revision; m_NextRequest = Plat_FloatTime() + 60.0; }
        }
        if ( !m_Worker.joinable() ) StartImageRequest();
    }
    void Shutdown()
    {
        m_Shutdown = true; m_Cancel.store( true );
        if ( m_Worker.joinable() ) m_Worker.join();
        for ( const ImageAsset& asset : m_Images ) if ( asset.texture > 0 ) vgui::surface()->DestroyTextureID( asset.texture );
        m_Images.clear();
    }
    bool Loading() const { return !m_HasFeed && !m_Offline; }
    bool Offline() const { return m_Offline; }
    bool HasFeed() const { return m_HasFeed; }
    int Revision() const { return m_Revision; }
    const CUtlVector<NewsEntry>& Entries() const { return m_Entries; }
    const ImageAsset* ImageFor( const char* path ) const
    {
        for ( const ImageAsset& asset : m_Images ) if ( asset.path == path && asset.texture > 0 ) return &asset;
        return nullptr;
    }
private:
    std::string ImageCachePath( const std::string& path ) const
    {
        unsigned hash = 2166136261u;
        for ( unsigned char c : m_ModeId + m_Repository + m_Branch + path ) hash = ( hash ^ c ) * 16777619u;
        char name[80]; V_snprintf( name, sizeof( name ), "cfg/scp_news_images/%08x.dat", hash ); return name;
    }
    void RefreshImages()
    {
        std::vector<ImageAsset> next;
        for ( const NewsEntry& entry : m_Entries )
        {
            if ( entry.image.IsEmpty() || next.size() >= 8 ) continue;
            bool exists = false;
            for ( const ImageAsset& asset : next ) if ( asset.path == entry.image.Get() ) exists = true;
            if ( exists ) continue;
            ImageAsset asset; asset.path = entry.image.Get();
            for ( ImageAsset& old : m_Images ) if ( old.path == asset.path ) { asset = old; old.texture = -1; }
            asset.attempted = false;
            next.push_back( asset );
        }
        for ( const ImageAsset& old : m_Images ) if ( old.texture > 0 ) vgui::surface()->DestroyTextureID( old.texture );
        m_Images.swap( next );
    }
    void StartImageRequest()
    {
        for ( unsigned i = 0; i < m_Images.size(); ++i )
        {
            if ( m_Images[i].attempted ) continue;
            m_ImageJob = (int)i; m_Response.clear(); m_Done.store( false );
            FileHandle_t file = g_pFullFileSystem->Open( ImageCachePath( m_Images[i].path ).c_str(), "rb", "MOD" );
            if ( file != FILESYSTEM_INVALID_HANDLE )
            {
                unsigned size = g_pFullFileSystem->Size( file );
                if ( size && size <= 2 * 1024 * 1024 )
                {
                    m_Response.resize( size );
                    if ( g_pFullFileSystem->Read( &m_Response[0], size, file ) != (int)size ) m_Response.clear();
                }
                g_pFullFileSystem->Close( file );
            }
            if(m_Response.empty()) {
                FileHandle_t local=g_pFullFileSystem->Open(m_Images[i].path.c_str(),"rb","GAME");
                if(local!=FILESYSTEM_INVALID_HANDLE) {
                    unsigned size=g_pFullFileSystem->Size(local);
                    if(size && size<=2*1024*1024) { m_Response.resize(size); if(g_pFullFileSystem->Read(&m_Response[0],size,local)!=(int)size) m_Response.clear(); }
                    g_pFullFileSystem->Close(local);
                }
            }
            try
            {
                const bool refreshOnline = !m_Offline && !m_Repository.empty();
                m_Worker = std::thread( [this,i,refreshOnline]() {
                    try
                    {
                        std::string cached; cached.swap( m_Response );
                        m_Success = false;
                        if ( refreshOnline && !m_Cancel.load() )
                            m_Success = GamepadUINewsHTTP::DownloadImage( m_Images[i].path, m_Response, m_Status, m_Cancel, m_Repository, m_Branch ) && GamepadUINewsHTTP::DecodeImage( m_Response, m_DecodedImage );
                        if ( !m_Success && !cached.empty() )
                        {
                            m_Response.swap( cached );
                            m_Success = GamepadUINewsHTTP::DecodeImage( m_Response, m_DecodedImage );
                        }
                    }
                    catch ( ... ) { m_Success = false; }
                    m_Done.store( true );
                } );
            }
            catch ( ... ) { m_Images[i].attempted = true; m_ImageJob = -1; }
            break;
        }
    }
    bool ConfigureMode()
    {
        const int revision=GamepadUIModeRevision();
        const std::string id=GamepadUIGetModeId();
        if(m_ModeRevision==revision && m_ModeId==id) return true;
        // Cancel an old source without blocking the menu while WinHTTP is waiting.
        if(m_Worker.joinable()) { m_Cancel.store(true); if(!m_Done.load()) return false; m_Worker.join(); }
        for(const ImageAsset& asset:m_Images) if(asset.texture>0) vgui::surface()->DestroyTextureID(asset.texture);
        m_Images.clear(); m_Entries.RemoveAll(); m_Response.clear(); m_DecodedImage.rgba.clear(); m_ImageJob=-1;
        m_ModeRevision=revision; m_ModeId=id;
        KeyValues *mode=GamepadUIGetModeConfig();
        m_Repository=mode->GetString("news/repository"); m_Branch=mode->GetString("news/branch","main");
        m_FeedFile=mode->GetString("news/file","news.vdf"); m_LocalFile=mode->GetString("news/local_file",NEWS_LOCAL);
        unsigned hash=2166136261u; for(unsigned char c:m_Repository+m_Branch+m_FeedFile) hash=(hash^c)*16777619u;
        char cache[160]; V_snprintf(cache,sizeof(cache),"cfg/scp_news/%s_%08x.vdf",m_ModeId.c_str(),hash); m_CacheFile=cache;
        m_Initialized=false; m_HasFeed=false; m_Offline=m_Repository.empty(); m_Success=false; m_NextRequest=0;
        m_Cancel.store(false); m_Done.store(false); ++m_Revision;
        return true;
    }
    bool LoadFile( const char* path, const char* pathId )
    {
        FileHandle_t file = g_pFullFileSystem->Open( path, "rb", pathId );
        if ( file == FILESYSTEM_INVALID_HANDLE ) return false;
        unsigned size = g_pFullFileSystem->Size( file );
        std::string contents;
        bool valid = size > 0 && size <= GamepadUINewsHTTP::MAX_FEED_BYTES;
        if ( valid ) { contents.resize( size ); valid = g_pFullFileSystem->Read( &contents[0], size, file ) == (int)size; }
        g_pFullFileSystem->Close( file );
        if ( !valid || !ParseFeed( contents, m_Entries ) ) return false;
        m_HasFeed = true; ++m_Revision; return true;
    }
    CUtlVector<NewsEntry> m_Entries;
    std::thread m_Worker;
    std::atomic<bool> m_Cancel{ false }, m_Done{ false };
    std::string m_ModeId, m_Repository, m_Branch, m_FeedFile, m_LocalFile, m_CacheFile;
    int m_ModeRevision=-1;
    std::string m_Response;
    std::vector<ImageAsset> m_Images;
    GamepadUINewsHTTP::Image m_DecodedImage;
    int m_ImageJob = -1;
    unsigned m_Status = 0;
    bool m_Initialized = false, m_Shutdown = false, m_Success = false, m_HasFeed = false, m_Offline = false;
    int m_Revision = 0;
    double m_NextRequest = 0;
};
NewsService s_News;

bool Russian() { return !V_stricmp( GamepadUIGetLanguageCode(), "RU" ); }
const char* LocalizedField( const CUtlString& language, const CUtlString& fallback, const CUtlString& english )
{
    if ( !language.IsEmpty() ) return language.Get();
    if ( !fallback.IsEmpty() ) return fallback.Get();
    return english.Get();
}


class GamepadUINewsPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( GamepadUINewsPanel, vgui::Panel );
    struct Line
    {
        GamepadUIString text;
        int y = 0, font = 0;
        Color color;
    };
    struct Picture { int texture, y, width, height; };
public:
    GamepadUINewsPanel( vgui::Panel* parent ) : BaseClass( parent, "SCPNews" )
    {
        SetPaintBackgroundEnabled( false ); SetPaintBorderEnabled( false );
        SetKeyBoardInputEnabled( false ); SetMouseInputEnabled( true );
        s_News.Initialize();
    }
    void Paint() OVERRIDE
    {
        const bool russian = Russian();
        const bool resized = m_Width != GetWide() || m_Height != GetTall();
        if ( resized || !m_Fonts[0] ) ConfigureFonts();
        if ( resized || m_Revision != s_News.Revision() || russian != m_Russian )
        {
            bool resetScroll = russian != m_Russian;
            m_Width = GetWide(); m_Height = GetTall(); m_Revision = s_News.Revision(); m_Russian = russian;
            if ( resetScroll ) m_Scroll = 0;
            Rebuild();
        }
        auto* surface = vgui::surface();
        surface->DrawSetTextFont( m_Fonts[2] ); surface->DrawSetTextColor( 255, 255, 255, 255 );
        surface->DrawSetTextPos( m_Padding, m_Padding );
        const wchar_t* heading = russian ? L"\u041D\u041E\u0412\u041E\u0421\u0422\u0418" : L"NEWS";
        surface->DrawPrintText( heading, V_wcslen( heading ) );
        const int top = m_Padding + surface->GetFontTall( m_Fonts[2] ) + 16;
        const int bottom = GetTall() - m_Padding - 24;
        const int available = Max( 1, bottom - top );
        m_MaxScroll = Max( 0, m_ContentHeight - available ); m_Scroll = Clamp( m_Scroll, 0, m_MaxScroll );
        for ( const Picture& picture : m_Pictures )
        {
            int y = top + picture.y - m_Scroll;
            int clippedTop = Max( y, top ), clippedBottom = Min( y + picture.height, bottom );
            if ( clippedBottom <= clippedTop ) continue;
            surface->DrawSetColor( 255, 255, 255, 255 ); surface->DrawSetTexture( picture.texture );
            surface->DrawTexturedSubRect( m_Padding, clippedTop, m_Padding + picture.width, clippedBottom,
                0, float( clippedTop - y ) / picture.height, 1, float( clippedBottom - y ) / picture.height );
        }
        surface->DrawSetTexture( -1 );
        for ( const Line& line : m_Lines )
        {
            int y = top + line.y - m_Scroll;
            if ( y < top || y + surface->GetFontTall( m_Fonts[line.font] ) > bottom ) continue;
            surface->DrawSetTextFont( m_Fonts[line.font] ); surface->DrawSetTextColor( line.color );
            surface->DrawSetTextPos( m_Padding, y ); surface->DrawPrintText( line.text.String(), line.text.Length() );
        }
        if ( m_MaxScroll > 0 )
        {
            const int barHeight = Max( 24, available * available / Max( 1, m_ContentHeight ) );
            const int barY = top + (available - barHeight) * m_Scroll / m_MaxScroll;
            surface->DrawSetColor( 255, 255, 255, 110 );
            surface->DrawFilledRect( GetWide() - 6, barY, GetWide() - 3, barY + barHeight );
        }
        if ( s_News.Offline() )
        {
            const wchar_t* status = russian ? L"\u041D\u0435\u0442 \u0441\u0432\u044F\u0437\u0438" : L"Offline";
            surface->DrawSetTextFont( m_Fonts[3] ); surface->DrawSetTextColor( 155, 155, 155, 255 );
            surface->DrawSetTextPos( m_Padding, GetTall() - m_Padding - 18 ); surface->DrawPrintText( status, V_wcslen( status ) );
        }
    }
    void OnMouseWheeled( int delta ) OVERRIDE { m_Scroll = Clamp( m_Scroll - delta * 48, 0, m_MaxScroll ); Repaint(); }
private:
    void ConfigureFonts()
    {
        vgui::IScheme* scheme = vgui::scheme()->GetIScheme( GetParent()->GetScheme() );
        const char* names[] = { "News.Body.Font", "News.Entry.Font", "News.Heading.Font", "News.Date.Font" };
        for ( int i = 0; i < 4; ++i )
        {
            // Use the same scheme font loader as the working main-menu buttons.
            m_Fonts[i] = SCPUIGetTextFont( vgui::surface(), scheme, names[i], true );
            if ( !m_Fonts[i] ) m_Fonts[i] = SCPUIGetTextFont( vgui::surface(), scheme, "Button.Text.Font", true );
        }
        m_Padding = Max( 14, int( GetTall() * 0.022f ) );
    }
    void AddLine( const wchar_t* text, int length, int font, Color color )
    {
        Line line; line.text.SetText( text, length ); line.y = m_ContentHeight; line.font = font; line.color = color;
        m_Lines.push_back( line );
        m_ContentHeight += vgui::surface()->GetFontTall( m_Fonts[font] ) + Max( 3, GetTall() / 180 );
    }
    void Wrap( const wchar_t* text, int font, Color color )
    {
        const int maxWidth = Max( 1, GetWide() - m_Padding * 2 - 4 );
        const int length = V_wcslen( text );
        int from = 0, cursor = 0, lastSpace = -1, width = 0;
        while ( cursor < length )
        {
            wchar_t c = text[cursor];
            if ( c == L'\r' ) { ++cursor; continue; }
            if ( c == L'\n' )
            {
                AddLine( text + from, cursor - from, font, color );
                from = ++cursor; lastSpace = -1; width = 0; continue;
            }
            int charWidth = vgui::surface()->GetCharacterWidth( m_Fonts[font], c );
            if ( width + charWidth > maxWidth && cursor > from )
            {
                int end = lastSpace >= from ? lastSpace : cursor;
                AddLine( text + from, end - from, font, color );
                from = end;
                while ( from < length && (text[from] == L' ' || text[from] == L'\t') ) ++from;
                cursor = from; lastSpace = -1; width = 0; continue;
            }
            if ( c == L' ' || c == L'\t' ) lastSpace = cursor;
            width += charWidth; ++cursor;
        }
        if ( from < length ) AddLine( text + from, length - from, font, color );
    }
    void WrapUTF8( const char* text, int font, Color color )
    {
        std::vector<wchar_t> unicode( V_strlen( text ) + 1 );
        V_UTF8ToUnicode( text, unicode.data(), (int)(unicode.size() * sizeof( wchar_t )) );
        Wrap( unicode.data(), font, color );
    }
    void Rebuild()
    {
        m_Lines.clear(); m_Pictures.clear(); m_ContentHeight = 0;
        if ( s_News.Entries().IsEmpty() )
        {
            Wrap( s_News.Loading()
                ? ( m_Russian ? L"\u0417\u0430\u0433\u0440\u0443\u0437\u043A\u0430 \u043D\u043E\u0432\u043E\u0441\u0442\u0435\u0439..." : L"Loading news..." )
                : s_News.HasFeed()
                    ? ( m_Russian ? L"\u041F\u043E\u043A\u0430 \u043D\u043E\u0432\u043E\u0441\u0442\u0435\u0439 \u043D\u0435\u0442." : L"No news yet." )
                    : ( m_Russian ? L"\u041D\u0435 \u0443\u0434\u0430\u043B\u043E\u0441\u044C \u0437\u0430\u0433\u0440\u0443\u0437\u0438\u0442\u044C \u043D\u043E\u0432\u043E\u0441\u0442\u0438." : L"News is unavailable." ),
                0, Color( 180, 180, 180, 255 ) );
        }
        for ( const NewsEntry& entry : s_News.Entries() )
        {
            const auto* image = s_News.ImageFor( entry.image.Get() );
            if ( image && image->width && image->height )
            {
                int width = Max( 1, GetWide() - m_Padding * 2 - 4 );
                int height = width * image->height / image->width;
                Picture picture = { image->texture, m_ContentHeight, width, Max( 1, height ) };
                m_Pictures.push_back( picture ); m_ContentHeight += height + 16;
            }
            WrapUTF8( LocalizedField( m_Russian ? entry.titleRussian : entry.titleEnglish, entry.title, entry.titleEnglish.IsEmpty() ? entry.titleRussian : entry.titleEnglish ), 1, Color( 255, 255, 255, 255 ) );
            m_ContentHeight += 6;
            if ( !entry.date.IsEmpty() ) { WrapUTF8( entry.date.Get(), 3, Color( 160, 160, 160, 255 ) ); m_ContentHeight += 10; }
            WrapUTF8( LocalizedField( m_Russian ? entry.bodyRussian : entry.bodyEnglish, entry.body, entry.bodyEnglish.IsEmpty() ? entry.bodyRussian : entry.bodyEnglish ), 0, Color( 225, 225, 225, 255 ) );
            m_ContentHeight += 28;
        }
    }
    std::vector<Line> m_Lines;
    std::vector<Picture> m_Pictures;
    vgui::HFont m_Fonts[4] = {};
    int m_Revision = -1, m_Width = -1, m_Height = -1, m_Padding = 16;
    int m_ContentHeight = 0, m_Scroll = 0, m_MaxScroll = 0;
    bool m_Russian = false;
};

}

vgui::Panel* GamepadUICreateNewsPanel( vgui::Panel* parent ) { return new GamepadUINewsPanel( parent ); }
void GamepadUINewsUpdate() { s_News.Update(); }
void GamepadUINewsShutdown() { s_News.Shutdown(); }
