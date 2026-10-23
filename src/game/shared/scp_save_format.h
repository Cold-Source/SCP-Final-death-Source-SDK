#ifndef SCP_SAVE_FORMAT_H
#define SCP_SAVE_FORMAT_H
#include <stddef.h>
#include <string.h>

inline bool SCPSaveReadInt( const unsigned char *data, size_t size, size_t &offset, int &value )
{
    if ( offset > size || size - offset < 4 ) return false;
    memcpy( &value, data + offset, 4 ); offset += 4; return true;
}

inline bool SCPSafeSaveSlot( const char *slot )
{
    if ( !slot || !*slot || strlen( slot ) > 63 ) return false;
    for ( const char *p = slot; *p; ++p )
        if ( !( ( *p >= 'a' && *p <= 'z' ) || ( *p >= 'A' && *p <= 'Z' ) || ( *p >= '0' && *p <= '9' ) || *p == '_' || *p == '-' ) ) return false;
    return true;
}

// Check the complete engine save archive, not just its initial GameHeader.
// A partial write may contain a valid header but still lack an embedded HL file.
inline bool SCPCompleteSingleSave( const unsigned char *data, size_t size )
{
    size_t offset = 0; int magic, version, body, count, tokens;
    if ( !SCPSaveReadInt( data, size, offset, magic ) || magic != 0x5641534a ||
         !SCPSaveReadInt( data, size, offset, version ) || version != 0x73 ||
         !SCPSaveReadInt( data, size, offset, body ) || body <= 0 ||
         !SCPSaveReadInt( data, size, offset, count ) || count <= 0 || count > 65535 ||
         !SCPSaveReadInt( data, size, offset, tokens ) || tokens <= 0 ) return false;
    if ( (size_t)body > size - offset ) return false;
    offset += body;
    if ( (size_t)tokens > size - offset ) return false;
    offset += tokens;
    if ( !SCPSaveReadInt( data, size, offset, count ) || count <= 0 || count > 4096 ) return false;
    for ( int i = 0; i < count; ++i )
    {
        if ( size - offset < 260 || !data[offset] || !memchr( data + offset, 0, 260 ) ) return false;
        offset += 260;
        int length;
        if ( !SCPSaveReadInt( data, size, offset, length ) || length < 0 || (size_t)length > size - offset ) return false;
        offset += length;
    }
    return offset == size;
}

struct SCPCoopSaveHeader
{
    char map[96];
    float seconds;
    int players;
};

inline bool SCPReadCoopSaveHeader( const unsigned char *data, size_t size, SCPCoopSaveHeader &header )
{
    size_t offset = 0; int magic, version, pointerSize;
    if ( !SCPSaveReadInt( data, size, offset, magic ) || magic != 0x504d5053 ||
         !SCPSaveReadInt( data, size, offset, version ) || version != 1 ||
         !SCPSaveReadInt( data, size, offset, pointerSize ) || pointerSize != 8 ) return false;
    size_t n = 0;
    while ( offset < size && data[offset] && n + 1 < sizeof( header.map ) )
    {
        const unsigned char c = data[offset++];
        if ( !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '/' ) ) return false;
        header.map[n++] = c;
    }
    if ( !n || offset >= size || data[offset++] || header.map[0] == '/' ) return false;
    header.map[n] = 0;
    int bits; if ( !SCPSaveReadInt( data, size, offset, bits ) ) return false;
    memcpy( &header.seconds, &bits, 4 );
    int body, headers, bytes;
    return SCPSaveReadInt( data, size, offset, body ) && SCPSaveReadInt( data, size, offset, headers ) &&
        SCPSaveReadInt( data, size, offset, bytes ) && SCPSaveReadInt( data, size, offset, header.players ) &&
        body >= 0 && headers > body && bytes > headers && bytes <= 64 * 1024 * 1024 && header.players >= 0 && header.players <= 256;
}
#endif
