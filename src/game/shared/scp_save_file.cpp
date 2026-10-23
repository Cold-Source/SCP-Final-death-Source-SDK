#include "cbase.h"
#include "filesystem.h"
#include "tier1/utlbuffer.h"
#include "tier1/strtools.h"
#include "scp_save_file.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include "tier0/memdbgon.h"

bool SCPWriteSaveAtomic( IFileSystem *fs, const char *path, CUtlBuffer &buffer )
{
    if ( !buffer.IsValid() ) return false;
    char temporary[MAX_PATH]; V_snprintf( temporary, sizeof( temporary ), "%s.tmp", path );
    // The original save is never opened for writing.
    if ( !fs->WriteFile( temporary, "MOD", buffer ) || fs->Size( temporary, "MOD" ) != (unsigned)buffer.TellPut() ) return false;
#ifdef _WIN32
    char source[MAX_PATH], target[MAX_PATH];
    if ( !fs->RelativePathToFullPath( temporary, "MOD", source, sizeof( source ) ) ) return false;
    V_strncpy( target, source, sizeof( target ) );
    target[V_strlen( target ) - 4] = 0;
    return MoveFileExA( source, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) != FALSE;
#else
    return fs->RenameFile( temporary, path, "MOD" );
#endif
}
