// Standalone Windows bootstrap. The engine caches its GamepadUI mode before
// client.dll is loaded, so this mode must be supplied at process creation.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

namespace
{
std::wstring QuoteArgument( const std::wstring& argument )
{
    std::wstring quoted = L"\"";
    size_t backslashes = 0;
    for ( wchar_t character : argument )
    {
        if ( character == L'\\' )
        {
            ++backslashes;
            continue;
        }
        quoted.append( character == L'"' ? 2 * backslashes + 1 : backslashes, L'\\' );
        backslashes = 0;
        quoted += character;
    }
    quoted.append( 2 * backslashes, L'\\' );
    quoted += L'"';
    return quoted;
}

int ShowStartupError( const std::wstring& message, DWORD error = 0 )
{
    std::wstring text( message );
    if ( error )
    {
        wchar_t* details = nullptr;
        if ( FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                             FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, 0,
                             reinterpret_cast<wchar_t*>( &details ), 0, nullptr ) && details )
        {
            text += L"\n\n";
            text += details;
            LocalFree( details );
        }
    }
    MessageBoxW( nullptr, text.c_str(), L"SCP: Final Death", MB_OK | MB_ICONERROR );
    return 1;
}
}

int WINAPI WinMain( HINSTANCE instance, HINSTANCE previousInstance, LPSTR rawCommandLine, int showWindow )
{
    std::vector<wchar_t> executablePath( 32768 );
    const DWORD length = GetModuleFileNameW( nullptr, executablePath.data(),
                                            static_cast<DWORD>( executablePath.size() ) );
    if ( !length || length >= executablePath.size() )
        return ShowStartupError( L"Could not determine the game directory.", GetLastError() );

    const std::wstring path( executablePath.data(), length );
    const size_t separator = path.find_last_of( L"\\/" );
    if ( separator == std::wstring::npos )
        return ShowStartupError( L"Could not determine the game directory." );
    const std::wstring directory = path.substr( 0, separator );
    int argumentCount = 0;
    wchar_t** arguments = CommandLineToArgvW( GetCommandLineW(), &argumentCount );
    if ( !arguments )
        return ShowStartupError( L"Could not read the launch arguments.", GetLastError() );

    bool gameSpecified = false, gamepadSpecified = false, gamepadDisabled = false;
    std::wstring commandLine = QuoteArgument( path );
    for ( int i = 1; i < argumentCount; ++i )
    {
        gameSpecified |= !_wcsicmp( arguments[i], L"-game" );
        gamepadSpecified |= !_wcsicmp( arguments[i], L"-gamepadui" );
        gamepadDisabled |= !_wcsicmp( arguments[i], L"-nogamepadui" );
        commandLine += L" " + QuoteArgument( arguments[i] );
    }
    LocalFree( arguments );

    if ( !gameSpecified || ( !gamepadSpecified && !gamepadDisabled ) )
    {
        // Engine mode is cached before client.dll loads. Restart this same EXE
        // once so the defaults are present in its real process command line.
        if ( !gameSpecified )
            commandLine += L" -game hl2";
        if ( !gamepadSpecified && !gamepadDisabled )
            commandLine += L" -gamepadui";
        if ( commandLine.size() >= 32767 )
            return ShowStartupError( L"The launch command line is too long." );

        std::vector<wchar_t> mutableCommandLine( commandLine.begin(), commandLine.end() );
        mutableCommandLine.push_back( L'\0' );
        STARTUPINFOW startup = {};
        startup.cb = sizeof( startup );
        startup.dwFlags = STARTF_USESHOWWINDOW;
        startup.wShowWindow = static_cast<WORD>( showWindow );
        PROCESS_INFORMATION process = {};
        if ( !CreateProcessW( path.c_str(), mutableCommandLine.data(), nullptr, nullptr,
                             FALSE, 0, nullptr, directory.c_str(), &startup, &process ) )
            return ShowStartupError( L"Could not start the game.", GetLastError() );

        CloseHandle( process.hThread );
        // Keep the parent alive for launcher/Steam tracking and propagate exit status.
        WaitForSingleObject( process.hProcess, INFINITE );
        DWORD exitCode = 1;
        GetExitCodeProcess( process.hProcess, &exitCode );
        CloseHandle( process.hProcess );
        return static_cast<int>( exitCode );
    }

    const std::wstring binDirectory = directory + L"\\bin";
    const std::wstring launcherPath = binDirectory + L"\\launcher.dll";
    if ( !SetCurrentDirectoryW( directory.c_str() ) || !SetDllDirectoryW( binDirectory.c_str() ) )
        return ShowStartupError( L"Could not set up the game directory.", GetLastError() );

    HMODULE launcher = LoadLibraryW( launcherPath.c_str() );
    if ( !launcher )
        return ShowStartupError( L"Cannot load " + launcherPath, GetLastError() );

    auto launcherMain = reinterpret_cast<int (WINAPI*)( HINSTANCE, HINSTANCE, LPSTR, int )>(
        GetProcAddress( launcher, "LauncherMain" ) );
    if ( !launcherMain )
    {
        FreeLibrary( launcher );
        return ShowStartupError( L"No LauncherMain in launcher.dll." );
    }
    return launcherMain( instance, previousInstance, rawCommandLine, showWindow );
}
