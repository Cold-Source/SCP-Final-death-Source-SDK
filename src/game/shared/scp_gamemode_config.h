#ifndef SCP_GAMEMODE_CONFIG_H
#define SCP_GAMEMODE_CONFIG_H
#include "tier1/KeyValues.h"
class IFileSystem;
inline bool SCPGamemodeIdentifier( const char *s )
{
    if ( !s || !*s || V_strlen(s) > 63 ) return false;
    for ( ; *s; ++s ) if ( !( (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') || *s == '_' || *s == '-' ) ) return false;
    return true;
}
bool SCPGamemodeAssetPath( const char *text );
// Caller owns the result. nullptr means an invalid/missing mode, never a partial config.
KeyValues *SCPReadGamemodeConfig( IFileSystem *fs, const char *id );
const char *SCPGamemodeText( KeyValues *config, const char *field, bool russian );
#endif
