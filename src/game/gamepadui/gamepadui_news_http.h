#ifndef GAMEPADUI_NEWS_HTTP_H
#define GAMEPADUI_NEWS_HTTP_H

#include <atomic>
#include <string>
#include <vector>

// The transport never touches VGUI, the engine filesystem or localization.
// It is called by a background worker and returns at most 256 KiB of text.
namespace GamepadUINewsHTTP
{
    const unsigned MAX_FEED_BYTES = 256 * 1024;
    bool Download( std::string& body, unsigned& status, const std::atomic<bool>& cancel );
    struct Image
    {
        std::vector<unsigned char> rgba;
        unsigned width = 0, height = 0;
    };
    bool IsImagePath( const std::string& path );
    bool DownloadImage( const std::string& path, std::string& encoded, unsigned& status, const std::atomic<bool>& cancel );
    bool DecodeImage( const std::string& encoded, Image& image );
}

#endif
