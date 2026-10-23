#ifndef SCP_PLAYERMODELS_H
#define SCP_PLAYERMODELS_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include "tier1/KeyValues.h"
#include "tier1/strtools.h"
#include "tier1/utlvector.h"

// The same allow-list is used by the preview browser and the server. Clients
// cannot select arbitrary models by putting a path in their userinfo.
struct SCPPlayerModel
{
    char path[MAX_PATH];
    char name[128];
    char nameRussian[128];
    char category[32];
};

inline bool SCPValidModelPath( const char *path )
{
    if ( !path || V_strnicmp( path, "models/", 7 ) || V_strlen( path ) >= MAX_PATH
        || V_stricmp( V_GetFileExtension( path ) ? V_GetFileExtension( path ) : "", "mdl" )
        || V_strstr( path, ".." ) )
        return false;
    for ( const char *p = path; *p; ++p )
        if ( (unsigned char)*p <= 32 || *p == '\\' || *p == '"' || *p == ';' || *p == ':' )
            return false;
    return true;
}

inline void SCPLoadPlayerModels( IFileSystem *fs, CUtlVector<SCPPlayerModel> &models )
{
    models.RemoveAll();
    KeyValues *data = new KeyValues( "SCPPlayerModels" );
    if ( data->LoadFromFile( fs, "scripts/scp_playermodels.txt", "GAME" ) )
    {
        for ( KeyValues *item = data->GetFirstTrueSubKey(); item && models.Count() < 128;
            item = item->GetNextTrueSubKey() )
        {
            const char *path = item->GetString( "model" );
            if ( !SCPValidModelPath( path ) || !fs->FileExists( path, "GAME" ) )
                continue;
            bool duplicate = false;
            FOR_EACH_VEC( models, i )
                if ( !V_stricmp( models[i].path, path ) ) duplicate = true;
            if ( duplicate ) continue;
            SCPPlayerModel model;
            V_strncpy( model.path, path, sizeof( model.path ) );
            V_strncpy( model.name, item->GetString( "name", item->GetName() ), sizeof( model.name ) );
            V_strncpy( model.nameRussian, item->GetString( "name_ru", model.name ), sizeof( model.nameRussian ) );
            V_strncpy( model.category, item->GetString( "category", "personnel" ), sizeof( model.category ) );
            models.AddToTail( model );
        }
    }
    data->deleteThis();
}

#endif
