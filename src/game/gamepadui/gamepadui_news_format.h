#ifndef GAMEPADUI_NEWS_FORMAT_H
#define GAMEPADUI_NEWS_FORMAT_H
#include <string>

namespace GamepadUINewsFormat
{
// Validate the small quoted KeyValues format before passing it to Source's
// permissive parser. Incomplete downloads must never replace the last good feed.
inline bool Validate( const std::string& text )
{
    size_t position = 0;
    if ( text.size() >= 3 && (unsigned char)text[0] == 0xef && (unsigned char)text[1] == 0xbb && (unsigned char)text[2] == 0xbf ) position = 3;
    bool pending[4] = {};
    bool rootStarted = false, rootClosed = false;
    int depth = 0;
    while ( position < text.size() )
    {
        const char c = text[position++];
        if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' ) continue;
        if ( c == '/' && position < text.size() && text[position] == '/' )
        {
            while ( position < text.size() && text[position] != '\n' ) ++position;
            continue;
        }
        if ( rootClosed ) return false;
        if ( c == '"' )
        {
            unsigned length = 0;
            bool closed = false;
            while ( position < text.size() )
            {
                const char value = text[position++];
                if ( value == '"' ) { closed = true; break; }
                if ( value == '\0' || (unsigned char)value < 32 ) return false;
                if ( value == '\\' )
                {
                    if ( position == text.size() ) return false;
                    const char escape = text[position++];
                    if ( escape != '\\' && escape != '"' && escape != 'n' && escape != 't' && escape != 'r' ) return false;
                }
                if ( ++length >= 4096 ) return false;
            }
            if ( !closed ) return false;
            if ( depth == 0 )
            {
                if ( rootStarted ) return false;
                rootStarted = true;
            }
            if ( !pending[depth] && !length ) return false;
            pending[depth] = !pending[depth];
        }
        else if ( c == '{' )
        {
            if ( !pending[depth] || depth >= 3 ) return false;
            pending[depth] = false; pending[++depth] = false;
        }
        else if ( c == '}' )
        {
            if ( !depth || pending[depth] ) return false;
            if ( --depth == 0 ) rootClosed = true;
        }
        else return false;
    }
    return rootClosed && depth == 0 && !pending[0];
}
}
#endif
