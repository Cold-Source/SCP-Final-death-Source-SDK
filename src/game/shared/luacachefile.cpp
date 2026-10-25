//========== Copyleft © 2011, Team Sandbox, Some rights reserved. ===========//
//
// Purpose: Handles the creation of Lua Cache Files for multiplayer.
//
//===========================================================================//

#include "cbase.h"
#include "filesystem.h"
#ifdef WIN32
#include "winlite.h"
#endif
#include "zip/XUnzip.h"
#include "utldict.h"
#include "tier1/utlbuffer.h"
#include "luamanager.h"
#include "luacachefile.h"

#ifdef GAME_DLL
#include "networkstringtable_gamedll.h"
#else
#include "networkstringtable_clientdll.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static IZip *s_lcfFile = 0;

//-----------------------------------------------------------------------------
// Purpose: // Get a lcffile instance
// Output : IZip*
//-----------------------------------------------------------------------------
LUA_API IZip* luasrc_GetLcfFile( void )
{
	if ( !s_lcfFile )
	{
		s_lcfFile = IZip::CreateZip();
	}
	return s_lcfFile;
}

//-----------------------------------------------------------------------------
// Purpose: Add file from disk to .lcf
// Input  : *relativename - 
//			*fullpath - 
//-----------------------------------------------------------------------------
LUA_API void luasrc_AddFileToLcf( const char *relativename, const char *fullpath )
{
	luasrc_GetLcfFile()->AddFileToZip( relativename, fullpath );
}

#ifdef CLIENT_DLL

static bool SafeClientPath(const char *path)
{
    if (!path || !path[0] || V_strlen(path)>=MAX_PATH || path[0]=='/' || path[0]=='\\'
        || V_strstr(path,"..") || V_strchr(path,':')) return false;
    for (const char *p=path; *p; ++p)
        if ((unsigned char)*p<32 || *p=='*' || *p=='?' || *p=='"' || *p=='|') return false;
    return true;
}
LUA_API void luasrc_ExtractLcf()
{
    INetworkStringTable *downloads=networkstringtable->FindTable("downloadables");
    if (!downloads) return;
    for (int n=0;n<downloads->GetNumStrings();++n)
    {
        const char *name=downloads->GetString(n);
        if (!SafeClientPath(name) || !V_GetFileExtension(name) || V_stricmp(V_GetFileExtension(name),"lcf")) continue;
        char archive[MAX_PATH];
        if (!filesystem->RelativePathToFullPath(name,"GAME",archive,sizeof(archive))) continue;
        HZIP zip=OpenZip(archive,0,ZIP_FILENAME);
        if (!zip) continue;
        ZIPENTRY entry;
        if (GetZipItem(zip,-1,&entry)!=ZR_OK || entry.index<0 || entry.index>4096) { CloseZip(zip); continue; }
        const int count=entry.index; int total=0;
        for (int i=0;i<count;++i)
        {
            if (GetZipItem(zip,i,&entry)!=ZR_OK) break;
            if (entry.attr & FILE_ATTRIBUTE_DIRECTORY) continue;
            if (!SafeClientPath(entry.name) || !V_GetFileExtension(entry.name) || V_stricmp(V_GetFileExtension(entry.name),"lua")
                || entry.unc_size<0 || entry.unc_size>4*1024*1024 || entry.unc_size>32*1024*1024-total)
            { Warning("Rejected invalid Lua cache entry\n"); break; }
            total+=entry.unc_size;
            CUtlBuffer bytes(0,entry.unc_size+1); bytes.EnsureCapacity(entry.unc_size+1);
            if (UnzipItem(zip,i,bytes.Base(),entry.unc_size+1,ZIP_MEMORY)!=ZR_OK) break;
            bytes.SeekPut(CUtlBuffer::SEEK_HEAD,entry.unc_size);
            char destination[MAX_PATH],directory[MAX_PATH];
            V_snprintf(destination,sizeof(destination),LUA_PATH_CACHE "/%s",entry.name);
            V_FixSlashes(destination,'/'); V_strncpy(directory,destination,sizeof(directory)); V_StripFilename(directory);
            filesystem->CreateDirHierarchy(directory,"MOD");
            if (!filesystem->WriteFile(destination,"MOD",bytes)) { Warning("Unable to write Lua cache entry\n"); break; }
        }
        CloseZip(zip);
    }
}

#else

static CUtlDict< char *, int > m_LcfDatabase;

bool luasrc_QueueClientFile(const char *input)
{
    if (gpGlobals->maxClients<=1) return true;
    if (!input || !input[0] || V_strlen(input)>=MAX_PATH-16 || input[0]=='/' || input[0]=='\\'
        || V_strstr(input,"..") || V_strchr(input,':')) return false;
    char path[MAX_PATH]; V_strncpy(path,input,sizeof(path)); V_FixSlashes(path,'/');
    for (const char *p=path; *p; ++p)
        if ((unsigned char)*p<32 || *p=='*' || *p=='?' || *p=='"' || *p=='|') return false;
    char full[MAX_PATH];
    if (!filesystem->FileExists(path,"MOD") || !filesystem->RelativePathToFullPath(path,"MOD",full,sizeof(full)))
    { Warning("Missing SCP client resource: %s\n",path); return false; }
    const char *ext=V_GetFileExtension(path);
    if (ext && !V_stricmp(ext,"lua"))
    {
        int index=m_LcfDatabase.Find(path);
        if (index!=m_LcfDatabase.InvalidIndex())
        { free(m_LcfDatabase[index]); m_LcfDatabase[index]=strdup(full); }
        else m_LcfDatabase.Insert(path,strdup(full));
        return true;
    }
    const char *allowed[]={"mdl","vvd","vtx","phy","vtf","vmt","wav"};
    bool accepted=false;
    for (int i=0;ext && i<ARRAYSIZE(allowed);++i) if (!V_stricmp(ext,allowed[i])) accepted=true;
    if (!accepted || (V_strnicmp(path,"models/",7) && V_strnicmp(path,"materials/",10) && V_strnicmp(path,"sound/",6))) return false;
    INetworkStringTable *downloads=networkstringtable->FindTable("downloadables");
    if (!downloads) return false;
    downloads->AddString(true,path);
    return true;
}

static int luasrc_sendfile(lua_State *L)
{
    lua_Debug caller;
    if (!lua_getstack(L,1,&caller) || !lua_getinfo(L,"S",&caller) || caller.source[0]!='@') return 0;
    char directory[MAX_PATH],full[MAX_PATH],relative[MAX_PATH];
    V_strncpy(directory,caller.source+1,sizeof(directory)); V_StripFilename(directory);
    const char *file=luaL_checkstring(L,1);
    if (V_strstr(file,"..") || V_strchr(file,':') || file[0]=='/' || file[0]=='\\') return luaL_error(L,"sendfile requires a relative path");
    V_snprintf(full,sizeof(full),"%s/%s",directory,file);
    if (!filesystem->FullPathToRelativePathEx(full,"MOD",relative,sizeof(relative)) || !luasrc_QueueClientFile(relative))
        return luaL_error(L,"sendfile: file cannot be sent");
    return 0;
}


static const luaL_Reg lcf_funcs[] = {
  {"sendfile", luasrc_sendfile},
  {NULL, NULL}
};


#endif

extern void lcf_open (lua_State *L) {
#ifndef CLIENT_DLL
  /* open lib into global table */
  luaL_register(L, "_G", lcf_funcs);
  lua_pop(L, 1);
#else
  // force create this directory incase it doesn't exist
  filesystem->CreateDirHierarchy( LUA_PATH_CACHE, "MOD");
#endif
}

extern void lcf_recursivedeletefile( const char *current ) {
	FileFindHandle_t fh;

	char path[ 512 ];
	if ( current[ 0 ] )
	{
        Q_snprintf( path, sizeof( path ), "%s/*.*", current );
	}
	else
	{
		Q_snprintf( path, sizeof( path ), "*.*" );
	}

	Q_FixSlashes( path );

	char const *fn = g_pFullFileSystem->FindFirstEx( path, "MOD", &fh );
	while ( fn )
	{
		if ( fn[0] != '.' )
		{
			if ( g_pFullFileSystem->FindIsDirectory( fh ) )
			{
				char nextdir[ 512 ];
				if ( current[ 0 ] )
				{
					Q_snprintf( nextdir, sizeof( nextdir ), "%s/%s", current, fn );
				}
				else
				{
					Q_snprintf( nextdir, sizeof( nextdir ), "%s", fn );
				}

				lcf_recursivedeletefile( nextdir );
			}
			else
			{
				char relative[ 512 ];
				if ( current[ 0 ] )
				{
					Q_snprintf( relative, sizeof( relative ), "%s/%s", current, fn );
				}
				else
				{
					Q_snprintf( relative, sizeof( relative ), "%s", fn );
				}
				DevMsg( "Deleting '%s/%s'...\n", current, fn );

				Q_FixSlashes( relative );
				g_pFullFileSystem->SetFileWritable( relative, true, "MOD" );
				g_pFullFileSystem->RemoveFile( relative, "MOD" );
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}
	g_pFullFileSystem->FindClose( fh );
}

extern void lcf_close (lua_State *L) {
#ifndef CLIENT_DLL
	int c = m_LcfDatabase.Count(); 
	for ( int i = 0; i < c; ++i )
	{
		free(m_LcfDatabase[ i ]);
	}
	m_LcfDatabase.RemoveAll();
#else
	lcf_recursivedeletefile( LUA_PATH_CACHE );
#endif
}

#ifndef CLIENT_DLL

extern void lcf_preparecachefile (void) {
	int c = m_LcfDatabase.Count();
	if ( c > 0 )
	{
		DevMsg( "Preparing Lua cache file...\n" );
	}
	else
	{
		return;
	}
	IZip *pZip = luasrc_GetLcfFile();
	for ( int i = 0; i < c; i++ )
	{
		pZip->AddFileToZip( m_LcfDatabase.GetElementName( i ), m_LcfDatabase[ i ] ); 
	}
	// force create this directory incase it doesn't exist
	filesystem->CreateDirHierarchy( "cache", "MOD");
    CUtlBuffer archive;
    pZip->SaveToBuffer(archive);
    const int size = archive.TellPut();
    if (size <= 0 || size > 64 * 1024 * 1024)
    {
        Warning("Invalid Lua cache archive size\n");
        return;
    }
    if (!filesystem->WriteFile("cache/cache_temp.lcf", "MOD", archive))
    {
        Warning("Cannot write Lua cache archive\n");
        return;
    }

    CRC32_t crc;
    CRC32_Init(&crc);
    CRC32_ProcessBuffer(&crc, archive.Base(), size);
    CRC32_Final(&crc);

	char filename[ MAX_PATH ];
	char hexname[ 16 ];
	Q_binarytohex( (const byte *)&crc, sizeof( crc ), hexname, sizeof( hexname ) );
	Q_snprintf( filename, sizeof( filename ), "cache\\%s.lcf", hexname );
	if ( g_pFullFileSystem->FileExists( filename, "MOD" ) )
	{
		g_pFullFileSystem->RemoveFile( "cache\\cache_temp.lcf", "MOD" );
	}
	else
	{
		g_pFullFileSystem->RenameFile( "cache\\cache_temp.lcf", filename, "MOD" );
	}

	INetworkStringTable *downloadables = networkstringtable->FindTable( "downloadables" );
    if (downloadables) downloadables->AddString(true,filename,-1);
	IZip::ReleaseZip( pZip );
	s_lcfFile = 0;
}

CON_COMMAND(dumpluacachefile, "Dump the contents of the Lua cache file database to the console.")
{
	int c = m_LcfDatabase.Count();
	for ( int i = 0; i < c; i++ )
	{
		Msg( "%s: %s\n", m_LcfDatabase.GetElementName( i ), m_LcfDatabase[ i ] );
	}
}

#endif
