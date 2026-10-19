#include "gamepadui_news_http.h"
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winhttp.h>
#include <wincodec.h>

namespace
{
class InternetHandle
{
public:
    explicit InternetHandle( HINTERNET handle ) : m_Handle( handle ) {}
    ~InternetHandle() { if ( m_Handle ) WinHttpCloseHandle( m_Handle ); }
    operator HINTERNET() const { return m_Handle; }
private:
    InternetHandle( const InternetHandle& ) = delete;
    InternetHandle& operator=( const InternetHandle& ) = delete;
    HINTERNET m_Handle;
};
}
#endif

static bool DownloadPath( const std::string& path, unsigned limit, std::string& body, unsigned& status, const std::atomic<bool>& cancel )
{
    body.clear(); status = 0;
#ifdef _WIN32
    // A fixed HTTPS endpoint needs neither a GitHub account nor a token in the game.
    InternetHandle session( WinHttpOpen( L"SCP-Final-Death-News/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                         WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 ) );
    if ( !session || cancel.load() ) return false;
    WinHttpSetTimeouts( session, 3000, 4000, 4000, 4000 );
    InternetHandle connection( WinHttpConnect( session, L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0 ) );
    if ( !connection || cancel.load() ) return false;
    std::wstring resource = L"/Cold-Source/SCP-FD-News/main/";
    resource.append( path.begin(), path.end() );
    InternetHandle request( WinHttpOpenRequest( connection, L"GET", resource.c_str(),
                                               nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE ) );
    if ( !request || cancel.load() ) return false;
    // No redirects to arbitrary hosts; retain the platform's normal TLS validation.
    DWORD redirects = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    WinHttpSetOption( request, WINHTTP_OPTION_REDIRECT_POLICY, &redirects, sizeof( redirects ) );
    const auto started = std::chrono::steady_clock::now();
    if ( !WinHttpSendRequest( request, L"Accept: text/plain\r\nCache-Control: no-cache\r\n", DWORD( -1 ),
                              WINHTTP_NO_REQUEST_DATA, 0, 0, 0 ) ||
         !WinHttpReceiveResponse( request, nullptr ) || cancel.load() ) return false;
    DWORD response = 0, size = sizeof( response );
    if ( !WinHttpQueryHeaders( request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                               WINHTTP_HEADER_NAME_BY_INDEX, &response, &size, WINHTTP_NO_HEADER_INDEX ) ) return false;
    status = response;
    if ( response != 200 ) return false;
    char buffer[8192];
    for ( ;; )
    {
        if ( cancel.load() || std::chrono::steady_clock::now() - started > std::chrono::seconds( 15 ) ) return false;
        DWORD received = 0;
        if ( !WinHttpReadData( request, buffer, sizeof( buffer ), &received ) ) return false;
        if ( !received ) return !body.empty();
        if ( body.size() + received > limit ) return false;
        body.append( buffer, received );
    }
#else
    // Cached/local news remain readable on platforms without this HTTPS transport.
    return false;
#endif
}

bool GamepadUINewsHTTP::Download( std::string& body, unsigned& status, const std::atomic<bool>& cancel )
{
    return DownloadPath( "news.vdf", MAX_FEED_BYTES, body, status, cancel );
}

bool GamepadUINewsHTTP::IsImagePath( const std::string& path )
{
    if ( path.empty() || path.size() > 240 || path[0] == '/' || path.find( ".." ) != std::string::npos ) return false;
    for ( unsigned char c : path )
        if ( !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '/' || c == '.' ) ) return false;
    const auto dot = path.find_last_of( '.' );
    if ( dot == std::string::npos ) return false;
    std::string extension = path.substr( dot );
    for ( char& c : extension ) if ( c >= 'A' && c <= 'Z' ) c += 'a' - 'A';
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg";
}

bool GamepadUINewsHTTP::DownloadImage( const std::string& path, std::string& encoded, unsigned& status, const std::atomic<bool>& cancel )
{
    return IsImagePath( path ) && DownloadPath( path, 2 * 1024 * 1024, encoded, status, cancel );
}

bool GamepadUINewsHTTP::DecodeImage( const std::string& encoded, Image& image )
{
    image.rgba.clear(); image.width = image.height = 0;
#ifdef _WIN32
    if ( encoded.empty() || encoded.size() > 2 * 1024 * 1024 ) return false;
    const HRESULT initialized = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    if ( FAILED( initialized ) && initialized != RPC_E_CHANGED_MODE ) return false;
    IWICImagingFactory* factory = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool success = false;
    do
    {
        if ( FAILED( CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &factory ) ) ) ) break;
        if ( FAILED( factory->CreateStream( &stream ) ) || FAILED( stream->InitializeFromMemory( (BYTE*)encoded.data(), (DWORD)encoded.size() ) ) ) break;
        if ( FAILED( factory->CreateDecoderFromStream( stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder ) ) ) break;
        if ( FAILED( decoder->GetFrame( 0, &frame ) ) ) break;
        UINT width = 0, height = 0;
        if ( FAILED( frame->GetSize( &width, &height ) ) || !width || !height || width > 4096 || height > 4096 ) break;
        IWICBitmapSource* source = frame;
        if ( width > 1024 || height > 1024 )
        {
            const unsigned largest = width > height ? width : height;
            width = (std::max)( 1u, width * 1024 / largest ); height = (std::max)( 1u, height * 1024 / largest );
            if ( FAILED( factory->CreateBitmapScaler( &scaler ) ) || FAILED( scaler->Initialize( frame, width, height, WICBitmapInterpolationModeFant ) ) ) break;
            source = scaler;
        }
        if ( FAILED( factory->CreateFormatConverter( &converter ) ) ||
             FAILED( converter->Initialize( source, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom ) ) ) break;
        image.rgba.resize( width * height * 4 );
        if ( FAILED( converter->CopyPixels( nullptr, width * 4, (UINT)image.rgba.size(), image.rgba.data() ) ) ) { image.rgba.clear(); break; }
        image.width = width; image.height = height; success = true;
    } while ( false );
    if ( converter ) converter->Release();
    if ( scaler ) scaler->Release();
    if ( frame ) frame->Release();
    if ( decoder ) decoder->Release();
    if ( stream ) stream->Release();
    if ( factory ) factory->Release();
    if ( SUCCEEDED( initialized ) ) CoUninitialize();
    return success;
#else
    return false;
#endif
}
