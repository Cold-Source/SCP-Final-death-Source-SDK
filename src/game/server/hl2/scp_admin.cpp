#include "cbase.h"
#include "scp_admin.h"
#include "../../shared/scp_admin_catalog.h"
#include "ai_basenpc.h"
#include "ammodef.h"
#include "luamanager.h"
#include "tier0/memdbgon.h"

namespace
{
void Reply( CBasePlayer *player, const char *text )
{
    ClientPrint( player, HUD_PRINTTALK, text );
}

CON_COMMAND(scp_weapon_list,"List weapons registered by the active SCP Lua content.")
{
    if(!g_bLuaInitialized || !L) { Msg("Load a map first.\n"); return; }
    const int top=lua_gettop(L);
    lua_getglobal(L,"weapon"); lua_getfield(L,-1,"getweapons");
    if(luasrc_pcall(L,0,1,0)==0 && lua_istable(L,-1)) {
        lua_pushnil(L);
        while(lua_next(L,-2)) {
            const char *name=lua_tostring(L,-2);
            if(name) Msg("%s\n",name);
            lua_pop(L,1);
        }
    }
    lua_settop(L,top);
}

void GiveWeapon( CBasePlayer *player, int index )
{
    if ( !player->IsAlive() ) return;
    const bool allow = CBaseEntity::IsPrecacheAllowed();
    CBaseEntity::SetAllowPrecache( true );
    player->GiveNamedItem( SCPAdminWeapons[index].classname );
    CBaseEntity::SetAllowPrecache( allow );
}

// Catalog entries are presentation only. Revalidate each custom class on the server.
bool RegisteredSWEP(const char *name)
{
    if (!SCPAdminWeaponClassname(name) || !L || !g_bLuaInitialized) return false;
    const int top = lua_gettop(L);
    bool valid = false;
    lua_getglobal(L, "weapon");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "get");
        lua_pushstring(L, name);
        if (lua_isfunction(L, -2) && luasrc_pcall(L, 1, 1, 0) == 0 && lua_istable(L, -1))
        {
            lua_getfield(L, -1, "swep_config"); valid = lua_istable(L, -1); lua_pop(L, 1);
            lua_getfield(L, -1, "Spawnable");
            if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) valid = false;
        }
    }
    lua_settop(L, top);
    return valid;
}

void GiveSWEP(CBasePlayer *player, const char *name)
{
    if (!player->IsAlive() || !RegisteredSWEP(name)) return;
    const bool allow = CBaseEntity::IsPrecacheAllowed();
    CBaseEntity::SetAllowPrecache(true);
    player->GiveNamedItem(name);
    CBaseEntity::SetAllowPrecache(allow);
}

// Find a free standing position near the destination, never inside a player/wall.
bool TeleportNear( CBasePlayer *moved, CBasePlayer *destination )
{
    if ( !moved->IsAlive() || !destination->IsAlive() || moved->IsInAVehicle() ) return false;
    const Vector offsets[] = { Vector( 80, 0, 16 ), Vector( -80, 0, 16 ), Vector( 0, 80, 16 ), Vector( 0, -80, 16 ) };
    for ( int i = 0; i < ARRAYSIZE( offsets ); ++i )
    {
        const Vector start = destination->GetAbsOrigin() + offsets[i];
        trace_t floor, space;
        UTIL_TraceHull( start, start - Vector( 0, 0, 128 ), moved->WorldAlignMins(), moved->WorldAlignMaxs(), MASK_PLAYERSOLID, moved, COLLISION_GROUP_PLAYER_MOVEMENT, &floor );
        if ( floor.startsolid || floor.allsolid || floor.fraction == 1.0f ) continue;
        Vector origin = floor.endpos + Vector( 0, 0, 1 );
        UTIL_TraceHull( origin, origin, moved->WorldAlignMins(), moved->WorldAlignMaxs(), MASK_PLAYERSOLID, moved, COLLISION_GROUP_PLAYER_MOVEMENT, &space );
        if ( space.startsolid || space.allsolid ) continue;
        moved->Teleport( &origin, NULL, &vec3_origin );
        return true;
    }
    return false;
}

void SpawnNPC( CBasePlayer *player, int index )
{
    if ( !player->IsAlive() ) return;
    Vector direction; player->EyeVectors( &direction );
    trace_t aim;
    UTIL_TraceLine( player->EyePosition(), player->EyePosition() + direction * 1024, MASK_NPCSOLID, player, COLLISION_GROUP_NONE, &aim );
    if ( aim.fraction == 1.0f || aim.startsolid || aim.surface.flags & SURF_SKY )
    { Reply( player, "#SCPAdmin_NoSpawnSpace" ); return; }
    const SCPAdminSpawn &item = SCPAdminNPCs[index];
    const bool allow = CBaseEntity::IsPrecacheAllowed();
    CBaseEntity::SetAllowPrecache( true );
    CAI_BaseNPC *npc = dynamic_cast<CAI_BaseNPC *>( CreateEntityByName( item.classname ) );
    if ( npc )
    {
        npc->SetAbsOrigin( aim.endpos + aim.plane.normal * 32 + Vector( 0, 0, 8 ) );
        npc->SetAbsAngles( QAngle( 0, player->EyeAngles().y + 180, 0 ) );
        npc->KeyValue( "model", item.model );
        npc->KeyValue( "additionalequipment", item.equipment );
        if ( !V_stricmp( item.classname, "npc_citizen" ) ) npc->KeyValue( "citizentype", "3" );
        if ( DispatchSpawn( npc ) != -1 )
        {
            if ( !V_stricmp( item.classname, "npc_manhack" ) ) npc->SetAbsOrigin( npc->GetAbsOrigin() + Vector( 0, 0, 48 ) );
            else UTIL_DropToFloor( npc, MASK_NPCSOLID );
            trace_t space;
            UTIL_TraceHull( npc->GetAbsOrigin(), npc->GetAbsOrigin(), npc->GetHullMins(), npc->GetHullMaxs(), MASK_NPCSOLID, npc, COLLISION_GROUP_NONE, &space );
            if ( space.startsolid || space.allsolid )
            { UTIL_Remove( npc ); Reply( player, "#SCPAdmin_NoSpawnSpace" ); }
            else npc->Activate();
        }
    }
    CBaseEntity::SetAllowPrecache( allow );
}
}

bool SCPAdminCommand( CBasePlayer *player, const CCommand &args )
{
    if ( V_stricmp( args[0], "scp_admin" ) ) return false;
    ConVarRef cheats( "sv_cheats" );
    if ( !player || !cheats.IsValid() || !cheats.GetBool() || args.ArgC() < 2 ) return true;
    const char *action = args[1];
    if ( !V_stricmp( action, "health" ) && player->IsAlive() ) player->SetHealth( player->GetMaxHealth() );
    else if ( !V_stricmp( action, "armor" ) && player->IsAlive() ) player->SetArmorValue( 100 );
    else if ( !V_stricmp( action, "ammo" ) && player->IsAlive() )
    {
        for ( int i = 0; i < player->WeaponCount(); ++i )
        {
            CBaseCombatWeapon *weapon = player->GetWeapon( i );
            if ( !weapon ) continue;
            if ( weapon->GetPrimaryAmmoType() >= 0 ) player->GiveAmmo( 999, weapon->GetPrimaryAmmoType(), true );
            if ( weapon->GetSecondaryAmmoType() >= 0 ) player->GiveAmmo( 999, weapon->GetSecondaryAmmoType(), true );
        }
    }
    else if ( !V_stricmp( action, "god" ) && player->IsAlive() ) player->ToggleFlag( FL_GODMODE );
    else if ( !V_stricmp( action, "weapon" ) && args.ArgC() == 3 )
    {
        const int index = V_atoi( args[2] );
        if ( index >= 0 && index < ARRAYSIZE( SCPAdminWeapons ) ) GiveWeapon( player, index );
    }
    else if (!V_stricmp(action, "swep") && args.ArgC() == 3) GiveSWEP(player, args[2]);
    else
    {
        // Server-wide actions, NPC creation and moderation belong to the listen host.
        // Dedicated servers can still be administered through their own console/RCON.
        if ( engine->IsDedicatedServer() || player != UTIL_GetListenServerHost() )
        { Reply( player, "#SCPAdmin_HostOnly" ); return true; }
        if ( !V_stricmp( action, "npc" ) && args.ArgC() == 3 )
        {
            const int index = V_atoi( args[2] );
            if ( index >= 0 && index < ARRAYSIZE( SCPAdminNPCs ) ) SpawnNPC( player, index );
        }
        else if ( !V_stricmp( action, "allweapon" ) && args.ArgC() == 3 )
        {
            const int index = V_atoi( args[2] );
            if ( index >= 0 && index < ARRAYSIZE( SCPAdminWeapons ) )
                for ( int i = 1; i <= gpGlobals->maxClients; ++i )
                    if ( CBasePlayer *target = UTIL_PlayerByIndex( i ) ) GiveWeapon( target, index );
        }
        else if ( !V_stricmp( action, "setting" ) && args.ArgC() == 4 )
        {
            const int setting = V_atoi( args[2] );
            char *end = NULL;
            const float value = (float)strtod( args[3], &end );
            if ( !end || *end || end == args[3] || !IsFinite( value ) ) return true;
            const char *names[] = { "sv_gravity", "hl2_walkspeed", "hl2_normspeed", "hl2_sprintspeed", "scp_jump_multiplier" };
            const float lows[] = { 0, 1, 1, 1, 0.1f }, highs[] = { 2000, 1000, 1000, 1500, 5 };
            if ( setting < 0 || setting >= ARRAYSIZE( names ) ) return true;
            ConVarRef settingVar( names[setting] );
            if ( settingVar.IsValid() ) settingVar.SetValue( clamp( value, lows[setting], highs[setting] ) );
            if ( setting >= 1 && setting <= 3 )
            {
                ConVarRef maximum( "sv_maxspeed" ), walk( "hl2_walkspeed" ), normal( "hl2_normspeed" ), run( "hl2_sprintspeed" );
                maximum.SetValue( Max( walk.GetFloat(), Max( normal.GetFloat(), run.GetFloat() ) ) );
            }
        }
        else if ( !V_stricmp( action, "player" ) && args.ArgC() == 4 )
        {
            // User IDs avoid applying a stale UI row to a newly connected player.
            const int userID = V_atoi( args[3] );
            CBasePlayer *target = NULL;
            for ( int i = 1; i <= gpGlobals->maxClients; ++i )
            {
                CBasePlayer *candidate = UTIL_PlayerByIndex( i );
                if ( candidate && candidate->GetUserID() == userID ) { target = candidate; break; }
            }
            if ( !target ) return true;
            if ( !V_stricmp( args[2], "kick" ) && target != player )
                engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", target->GetUserID() ) );
            else if ( !V_stricmp( args[2], "kill" ) && target->IsAlive() )
            {
                target->RemoveFlag( FL_GODMODE );
                target->CommitSuicide( false, true );
            }
            else if ( !V_stricmp( args[2], "bring" ) && target != player )
            { if ( !TeleportNear( target, player ) ) Reply( player, "#SCPAdmin_NoTeleportSpace" ); }
            else if ( !V_stricmp( args[2], "goto" ) && target != player )
            { if ( !TeleportNear( player, target ) ) Reply( player, "#SCPAdmin_NoTeleportSpace" ); }
        }
    }
    return true;
}
