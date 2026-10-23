#include "scp_gamemode_config.h"
#include "filesystem.h"
#include "tier1/utlbuffer.h"
#include "lua.hpp"
#include <stdlib.h>
#include <math.h>

bool SCPGamemodeAssetPath( const char *s )
{
    if ( !s || !*s || V_strlen(s) > 240 || *s == '/' || V_strstr(s,"..") || V_strstr(s,"//") ) return false;
    for ( ; *s; ++s ) if ( !( (*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') || *s == '_' || *s == '-' || *s == '.' || *s == '/' ) ) return false;
    return true;
}
namespace {
struct MemoryBudget { size_t used; };
void *Allocate( void *ud, void *ptr, size_t oldSize, size_t newSize )
{
    MemoryBudget *budget = static_cast<MemoryBudget*>(ud);
    if ( !ptr ) oldSize = 0;
    if ( !newSize ) { free(ptr); budget->used -= oldSize; return nullptr; }
    if ( newSize > 2*1024*1024 || budget->used - oldSize > 2*1024*1024 - newSize ) return nullptr;
    void *next = realloc(ptr,newSize);
    if ( next ) budget->used = budget->used - oldSize + newSize;
    return next;
}
void Timeout( lua_State *L, lua_Debug * ) { luaL_error(L,"config.lua instruction budget exceeded"); }
// Only plain data enters engine/UI code. No libraries or native modules are opened.
bool Convert( lua_State *L, int index, KeyValues *out, int depth, int &nodes )
{
    if ( depth > 6 || ++nodes > 8192 ) return false;
    index = index > 0 ? index : lua_gettop(L)+index+1;
    switch ( lua_type(L,index) ) {
    case LUA_TBOOLEAN: out->SetInt(nullptr,lua_toboolean(L,index)); return true;
    case LUA_TNUMBER: {
        const double n=lua_tonumber(L,index);
        if ( n!=n || n < -1000000 || n > 1000000 ) return false;
        out->SetFloat(nullptr,(float)n); return true;
    }
    case LUA_TSTRING: {
        size_t length=0; const char *s=lua_tolstring(L,index,&length);
        if ( length > 8192 || V_strlen(s) != length ) return false;
        out->SetString(nullptr,s); return true;
    }
    case LUA_TTABLE: {
        const int count=(int)lua_objlen(L,index);
        if ( count > 512 ) return false;
        for ( int i=1;i<=count;++i ) {
            lua_rawgeti(L,index,i); char name[16]; V_snprintf(name,sizeof(name),"%d",i);
            KeyValues *child=new KeyValues(name); out->AddSubKey(child);
            const bool valid=Convert(L,-1,child,depth+1,nodes); lua_pop(L,1);
            if ( !valid ) return false;
        }
        lua_pushnil(L);
        while ( lua_next(L,index) ) {
            if ( lua_type(L,-2)==LUA_TNUMBER ) {
                double n=lua_tonumber(L,-2);
                if ( n < 1 || n > count || floor(n)!=n ) { lua_pop(L,2); return false; }
            } else if ( lua_type(L,-2)==LUA_TSTRING ) {
                size_t len=0; const char *name=lua_tolstring(L,-2,&len);
                if ( !len || len>63 || V_strlen(name)!=len || V_strstr(name,"/") || V_strstr(name,"\\") || (name[0]>='0' && name[0]<='9') ) { lua_pop(L,2); return false; }
                KeyValues *child=new KeyValues(name); out->AddSubKey(child);
                if ( !Convert(L,-1,child,depth+1,nodes) ) { lua_pop(L,2); return false; }
            } else { lua_pop(L,2); return false; }
            lua_pop(L,1);
        }
        return true;
    }
    default: return false;
    }
}
bool OptionalPath( KeyValues *config, const char *key )
{
    const char *s=config->GetString(key);
    return !*s || SCPGamemodeAssetPath(s);
}
bool Validate( KeyValues *c )
{
    if ( !OptionalPath(c,"background") || !OptionalPath(c,"logo") || !OptionalPath(c,"news/local_file") ||
         !OptionalPath(c,"news/repository") || !OptionalPath(c,"news/branch") || !OptionalPath(c,"news/file") ) return false;
    const char *repo=c->GetString("news/repository");
    const char *slash=V_strstr(repo,"/");
    if ( *repo && (!slash || slash==repo || !slash[1] || V_strstr(slash+1,"/")) ) return false;
    const int min=c->GetInt("min_players",c->GetBool("coop") ? 2 : 1);
    const int max=c->GetInt("max_players",c->GetBool("coop") ? 4 : 1);
    const int count=c->GetInt("default_players",max);
    if ( min<1 || max<min || max>4 || count<min || count>max || (!c->GetBool("coop") && max!=1) ) return false;
    c->SetInt("min_players",min); c->SetInt("max_players",max); c->SetInt("default_players",count);
    const char *lists[]={"maps","chapters"};
    for ( int i=0;i<2;++i ) {
        KeyValues *list=c->FindKey(lists[i]);
        for ( KeyValues *entry=list ? list->GetFirstSubKey() : nullptr;entry;entry=entry->GetNextKey() )
            if ( !SCPGamemodeIdentifier(entry->GetString("map")) || !OptionalPath(entry,"image") || entry->GetInt("number",1)<1 || entry->GetInt("number",1)>999 ) return false;
    }
    return true;
}
}
KeyValues *SCPReadGamemodeConfig( IFileSystem *fs, const char *id )
{
    if ( !fs || !SCPGamemodeIdentifier(id) ) return nullptr;
    char path[256]; V_snprintf(path,sizeof(path),"gamemodes/%s/gamemode/init.lua",id);
    if ( !fs->FileExists(path,"MOD") ) return nullptr;
    KeyValues *config=new KeyValues("SCPGamemode");
    config->SetString("id",id); config->SetString("name",id);

    V_snprintf(path,sizeof(path),"gamemodes/%s/gamemode/config.lua",id);
    if ( !fs->FileExists(path,"MOD") ) { config->SetBool("coop",true); config->SetBool("all_maps",true); Validate(config); return config; }
    config->Clear();
    CUtlBuffer source;
    FileHandle_t file=fs->Open(path,"rb","MOD");
    const int size=file==FILESYSTEM_INVALID_HANDLE ? 0 : fs->Size(file);
    bool valid=size>0 && size<=128*1024;
    if ( valid ) { source.EnsureCapacity(size); valid=fs->Read(source.Base(),size,file)==size; }
    if ( file!=FILESYSTEM_INVALID_HANDLE ) fs->Close(file);
    const char *text=valid ? (const char*)source.Base() : "";
    int length=valid ? size : 0;
    if(length>=3 && (unsigned char)text[0]==0xef && (unsigned char)text[1]==0xbb && (unsigned char)text[2]==0xbf) { text+=3; length-=3; }
    // This entry point accepts text configs, never binary Lua chunks.
    if(!length || (unsigned char)text[0]==0x1b || memchr(text,0,length)) valid=false;
    MemoryBudget budget={0}; lua_State *L=valid ? lua_newstate(Allocate,&budget) : nullptr;
    if ( L ) {
        lua_sethook(L,Timeout,LUA_MASKCOUNT,100000);
        valid=luaL_loadbuffer(L,text,length,path)==0 && lua_pcall(L,0,1,0)==0;
        int nodes=0;
        if ( valid ) valid=lua_istable(L,-1) && Convert(L,-1,config,0,nodes);
        else Warning("Gamemode %s: %s\n",id,lua_tostring(L,-1));
        lua_close(L);
    } else valid=false;
    // Set canonical identity after reading data; config cannot change its folder.
    config->SetString("id",id);
    if ( !config->FindKey("name") ) config->SetString("name",id);
    if ( !config->FindKey("all_maps") ) config->SetBool("all_maps",true);
    valid=valid && Validate(config);
    if ( !valid ) { Warning("Gamemode %s: invalid config.lua; mode skipped.\n",id); config->deleteThis(); return nullptr; }
    return config;
}
const char *SCPGamemodeText( KeyValues *config, const char *field, bool russian )
{
    char key[80]; V_snprintf(key,sizeof(key),"%s_ru",field);
    const char *text=russian ? config->GetString(key) : "";
    return *text ? text : config->GetString(field);
}
