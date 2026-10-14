#include "cbase.h"
#include "spmp_saveload.h"
#include "saverestore.h"
#include "saverestoretypes.h"
#include "entityapi.h"
#include "entitylist.h"
#include "globalstate.h"
#include "world.h"
#include "player.h"
#include "vehicle_base.h"
#include "igamesystem.h"
#include "tier1/utlbuffer.h"
#include "filesystem.h"
#include "steam/steam_api.h"
#include "tier0/memalloc.h"
#include "tier0/memdbgon.h"

extern void BeginRestoreEntities();
extern void EndRestoreEntities();

namespace
{
const int SAVE_MAGIC = 0x504D5053; // SPMP
const int SAVE_VERSION = 1;
const int SAVE_BUFFER_BYTES = 64 * 1024 * 1024;
struct SavedPlayer_t { int slot; uint64 steamID; };
CUtlVector<SavedPlayer_t> s_SavedPlayers;
bool s_RestoringWorld = false;
bool s_OwnSaveMemory = false;
struct CScopedSaveMemory
{
    CScopedSaveMemory() { s_OwnSaveMemory = true; }
    ~CScopedSaveMemory() { s_OwnSaveMemory = false; }
};

bool ValidSlot( const char *slot )
{
    if ( !slot || !*slot || Q_strlen(slot) > 64 ) return false;
    for ( const char *p = slot; *p; ++p )
        if ( !((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_') ) return false;
    return true;
}
bool HostCommand()
{
    CBasePlayer *player = UTIL_GetCommandClient();
    return !player || (!engine->IsDedicatedServer() && player->entindex() == 1);
}
uint64 PlayerID( CBasePlayer *player )
{
    const CSteamID *id = player ? engine->GetClientSteamID(player->edict()) : NULL;
    return id ? id->ConvertToUint64() : 0;
}
void ReleaseSaveData( CSaveRestoreData *data )
{
    if ( !data ) return;
    entitytable_t *table = data->DetachEntityTable();
    if ( table ) MemAlloc_Free(table);
    char **tokens = data->DetachSymbolTable();
    if ( tokens )
    {
        // Restore may add symbols that point to static field names. The table
        // owns only its pointer array; tokenStrings in LoadWorld owns its text.
        MemAlloc_Free(tokens);
    }
    data->~CSaveRestoreData();
    MemAlloc_Free(data);
}

bool SaveWorld( const char *slot )
{
    CSaveRestoreData *previous = gpGlobals->pSaveData;
    CSaveRestoreData *data = SaveInit(SAVE_BUFFER_BYTES);
    if ( !data ) { Warning("SPMP save: could not allocate save data.\n"); return false; }
    Q_strncpy(data->levelInfo.szCurrentMapName, STRING(gpGlobals->mapname), sizeof(data->levelInfo.szCurrentMapName));
    data->levelInfo.mapVersion = gpGlobals->mapversion;
    CSave save(data);
    SaveGlobalState(data);
    const int bodyOffset = data->GetCurPos();
    g_pGameSaveRestoreBlockSet->PreSave(data);
    g_pGameSaveRestoreBlockSet->Save(&save);
    const int headerOffset = data->GetCurPos();
    g_pGameSaveRestoreBlockSet->WriteSaveHeaders(&save);
    g_pGameSaveRestoreBlockSet->PostSave();

    CUtlBuffer output;
    output.PutInt(SAVE_MAGIC); output.PutInt(SAVE_VERSION); output.PutInt(sizeof(void *));
    output.PutString(STRING(gpGlobals->mapname)); output.PutFloat(gpGlobals->curtime);
    output.PutInt(bodyOffset); output.PutInt(headerOffset); output.PutInt(data->GetCurPos());
    CUtlVector<SavedPlayer_t> players;
    for ( int i = 1; i <= gpGlobals->maxClients; ++i )
    {
        CBasePlayer *player = UTIL_PlayerByIndex(i);
        if ( player ) { SavedPlayer_t entry = { i, PlayerID(player) }; players.AddToTail(entry); }
    }
    output.PutInt(players.Count());
    FOR_EACH_VEC(players, i) { output.PutInt(players[i].slot); output.Put(&players[i].steamID, sizeof(uint64)); }
    const int tokenCount = data->SizeSymbolTable();
    char **tokens = data->DetachSymbolTable();
    output.PutInt(tokenCount);
    for ( int i = 0; i < tokenCount; ++i ) output.PutString(tokens[i] ? tokens[i] : "");
    MemAlloc_Free(tokens);
    output.Put(data->GetBuffer(), data->GetCurPos());
    char path[MAX_PATH]; Q_snprintf(path, sizeof(path), "save/%s.spmp", slot);
    filesystem->CreateDirHierarchy("save", "MOD");
    const bool result = output.IsValid() && filesystem->WriteFile(path, "MOD", output);
    if ( !result ) Warning("SPMP save: file write failed (%s), valid %d, bytes %d.\n", path, output.IsValid(), output.TellPut());
    const int entities = data->NumEntities();
    ReleaseSaveData(data); gpGlobals->pSaveData = previous;
    if ( result ) Msg("SPMP_SAVE_OK %s entities %d players %d\n", slot, entities, players.Count());
    return result;
}

bool LoadWorld( const char *slot )
{
    char path[MAX_PATH]; Q_snprintf(path, sizeof(path), "save/%s.spmp", slot);
    CUtlBuffer input;
    if ( !filesystem->ReadFile(path, "MOD", input) || input.TellPut() > SAVE_BUFFER_BYTES + 1024 * 1024 ) return false;
    if ( input.GetInt() != SAVE_MAGIC || input.GetInt() != SAVE_VERSION || input.GetInt() != sizeof(void *) ) return false;
    char map[MAX_MAP_NAME]; input.GetString(map, sizeof(map));
    const float time = input.GetFloat();
    const int bodyOffset = input.GetInt(), headerOffset = input.GetInt(), bytes = input.GetInt();
    const int playerCount = input.GetInt();
    if ( playerCount < 0 || playerCount > MAX_PLAYERS || bytes <= 0 || bytes > SAVE_BUFFER_BYTES || bodyOffset < 0 || bodyOffset >= headerOffset || headerOffset >= bytes ) return false;
    if ( Q_stricmp(map, STRING(gpGlobals->mapname)) ) { Warning("Load map %s before loading this SPMP save.\n", map); return false; }
    s_SavedPlayers.RemoveAll();
    for ( int i = 0; i < playerCount; ++i )
    {
        SavedPlayer_t entry; entry.slot = input.GetInt(); input.Get(&entry.steamID, sizeof(uint64)); s_SavedPlayers.AddToTail(entry);
    }
    const int tokenCount = input.GetInt();
    if ( tokenCount <= 0 || tokenCount > 65535 ) return false;
    CUtlVector<CUtlString> tokenStrings;
    tokenStrings.SetCount(tokenCount);
    for ( int i = 0; i < tokenCount; ++i ) { char token[4096]; input.GetString(token, sizeof(token)); tokenStrings[i] = token; }
    if ( !input.IsValid() || input.TellGet() + bytes != input.TellPut() ) return false;
    CSaveRestoreData *previous = gpGlobals->pSaveData;
    CSaveRestoreData *data = SaveInit(bytes);
    if ( !data ) return false;
    data->Init(data->GetBuffer(), bytes);
    char **tokens = data->DetachSymbolTable();
    MemAlloc_Free(tokens);
    tokens = (char **)MemAlloc_Alloc(tokenCount * sizeof(char *));
    if ( !tokens ) { ReleaseSaveData(data); gpGlobals->pSaveData = previous; return false; }
    data->InitSymbolTable(tokens, tokenCount);
    for ( int i = 0; i < tokenCount; ++i )
    {
        if ( tokenStrings[i].IsEmpty() ) continue;
        tokens[i] = const_cast<char *>(tokenStrings[i].String());
    }
    input.Get(data->GetBuffer(), bytes);
    data->levelInfo.time = time;
    data->levelInfo.mapVersion = gpGlobals->mapversion;
    Q_strncpy(data->levelInfo.szCurrentMapName, map, sizeof(data->levelInfo.szCurrentMapName));
    BeginRestoreEntities(); s_RestoringWorld = true;
    for ( int i = 1; i <= gpGlobals->maxClients; ++i )
    {
        CBasePlayer *player = UTIL_PlayerByIndex(i);
        if ( player && player->IsInAVehicle() ) player->LeaveVehicle();
        if ( player ) player->VPhysicsDestroyObject();
    }
    for ( CBaseEntity *entity = gEntList.FirstEnt(); entity; )
    {
        CBaseEntity *next = gEntList.NextEnt(entity);
        if ( !entity->IsPlayer() && entity != GetWorldEntity() )
        {
            CPropVehicle *vehicle = dynamic_cast<CPropVehicle *>(entity);
            if ( vehicle ) vehicle->GetPhysics()->ShutdownController();
            UTIL_RemoveImmediate(entity);
        }
        entity = next;
    }
    gEntList.CleanupDeleteList();
    data->Seek(0); RestoreGlobalState(data);
    CRestore restore(data);
    g_pGameSaveRestoreBlockSet->PreRestore();
    restore.SetReadPos(headerOffset); g_pGameSaveRestoreBlockSet->ReadRestoreHeaders(&restore);
    restore.SetReadPos(bodyOffset); g_pGameSaveRestoreBlockSet->Restore(&restore, false);
    g_pGameSaveRestoreBlockSet->PostRestore();
    EndRestoreEntities(); s_RestoringWorld = false;
    const int entities = data->NumEntities();
    ReleaseSaveData(data); gpGlobals->pSaveData = previous;
    Msg("SPMP_LOAD_OK %s entities %d players %d\n", slot, entities, playerCount);
    return true;
}

class CSpmpSaveSystem : public CAutoGameSystemPerFrame
{
public:
    CSpmpSaveSystem() : CAutoGameSystemPerFrame("SPMP save/restore"), m_load(false) {}
    void Request(const char *slot, bool load) { m_slot = slot; m_load = load; }
    void FrameUpdatePreEntityThink() OVERRIDE
    {
        if ( m_slot.IsEmpty() ) return;
        CUtlString slot = m_slot; m_slot.Clear();
        CScopedSaveMemory ownMemory;
        const bool success = m_load ? LoadWorld(slot.String()) : SaveWorld(slot.String());
        if ( !success ) Warning("SPMP_%s_FAILED %s\n", m_load ? "LOAD" : "SAVE", slot.String());
    }
private:
    CUtlString m_slot;
    bool m_load;
} s_SaveSystem;
}

bool SpmpIsRestoringWorld() { return s_RestoringWorld; }
bool SpmpUsesOwnSaveMemory() { return s_OwnSaveMemory; }
CBasePlayer *SpmpPlayerForSavedSlot(int slot)
{
    FOR_EACH_VEC(s_SavedPlayers, i)
    {
        if ( s_SavedPlayers[i].slot != slot ) continue;
        if ( !s_SavedPlayers[i].steamID ) return UTIL_PlayerByIndex(slot);
        for ( int index = 1; index <= gpGlobals->maxClients; ++index )
        {
            CBasePlayer *player = UTIL_PlayerByIndex(index);
            if ( player && PlayerID(player) == s_SavedPlayers[i].steamID ) return player;
        }
    }
    return NULL;
}
CON_COMMAND(spmp_save, "Save server world and players to save/<slot>.spmp.")
{
    if ( args.ArgC() == 2 && ValidSlot(args[1]) && HostCommand() ) s_SaveSystem.Request(args[1], false);
}
CON_COMMAND(spmp_load, "Load a server world save on the same map.")
{
    if ( args.ArgC() == 2 && ValidSlot(args[1]) && HostCommand() ) s_SaveSystem.Request(args[1], true);
}
