//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#define lbasecombatweapon_shared_cpp

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "lbasecombatweapon_shared.h"
#include "lbaseentity_shared.h"
#include "lbaseplayer_shared.h"
#include "baseviewmodel_shared.h"
#include "hl2mp/weapon_hl2mpbase_scriptedweapon.h"
#include "mathlib/lvector.h"
#ifndef CLIENT_DLL
#include "recipientfilter.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/


LUA_API lua_CBaseCombatWeapon *lua_toweapon (lua_State *L, int idx) {
  CBaseHandle *hWeapon = dynamic_cast<CBaseHandle *>((CBaseHandle *)lua_touserdata(L, idx));
  if (hWeapon == NULL)
    return NULL;
  return dynamic_cast<lua_CBaseCombatWeapon *>(hWeapon->Get());
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushweapon (lua_State *L, lua_CBaseCombatWeapon *pWeapon) {
  CBaseHandle *hWeapon = (CBaseHandle *)lua_newuserdata(L, sizeof(CBaseHandle));
  hWeapon->Set((CBaseEntity *)pWeapon);
  luaL_getmetatable(L, "CBaseCombatWeapon");
  lua_setmetatable(L, -2);
}


LUALIB_API lua_CBaseCombatWeapon *luaL_checkweapon (lua_State *L, int narg) {
  lua_CBaseCombatWeapon *d = lua_toweapon(L, narg);
  if (d == NULL)  /* avoid extra test when d is not 0 */
    luaL_argerror(L, narg, "CBaseCombatWeapon expected, got NULL entity");
  return d;
}


static int CBaseCombatWeapon_AbortReload (lua_State *L) {
  luaL_checkweapon(L, 1)->AbortReload();
  return 0;
}

static int CBaseCombatWeapon_Activate (lua_State *L) {
  luaL_checkweapon(L, 1)->Activate();
  return 0;
}

static int CBaseCombatWeapon_ActivityListCount (lua_State *L) {
  int count = 0;
  luaL_checkweapon(L, 1)->ActivityList(count);
  lua_pushinteger(L, count);
  return 1;
}

static int CBaseCombatWeapon_ActivityOverride (lua_State *L) {
  bool required = luaL_checkboolean(L, 3);
  bool *pRequired = &required;
  lua_pushinteger(L, luaL_checkweapon(L, 1)->ActivityOverride((Activity)luaL_checkint(L, 2), pRequired));
  return 1;
}

static int CBaseCombatWeapon_AddViewKick (lua_State *L) {
  luaL_checkweapon(L, 1)->AddViewKick();
  return 0;
}

static int CBaseCombatWeapon_AllowsAutoSwitchFrom (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->AllowsAutoSwitchFrom());
  return 1;
}

static int CBaseCombatWeapon_AllowsAutoSwitchTo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->AllowsAutoSwitchTo());
  return 1;
}

static int CBaseCombatWeapon_CalcViewmodelBob (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->CalcViewmodelBob());
  return 1;
}

static int CBaseCombatWeapon_CanBePickedUpByNPCs (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->CanBePickedUpByNPCs());
  return 1;
}

static int CBaseCombatWeapon_CanBeSelected (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->CanBeSelected());
  return 1;
}

static int CBaseCombatWeapon_CanDeploy (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->CanDeploy());
  return 1;
}

static int CBaseCombatWeapon_CanHolster (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->CanHolster());
  return 1;
}

static int CBaseCombatWeapon_CanLower (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->CanLower());
  return 1;
}

static int CBaseCombatWeapon_CheckReload (lua_State *L) {
  luaL_checkweapon(L, 1)->CheckReload();
  return 0;
}

static int CBaseCombatWeapon_Clip1 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->Clip1());
  return 1;
}

static int CBaseCombatWeapon_Clip2 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->Clip2());
  return 1;
}

static int CBaseCombatWeapon_DefaultDeploy (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->DefaultDeploy( (char*)luaL_checkstring(L, 2), (char*)luaL_checkstring(L, 3), luaL_checkint(L, 4), (char*)luaL_checkstring(L, 5) ));
  return 1;
}

static int CBaseCombatWeapon_DefaultReload (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->DefaultReload(luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4)));
  return 1;
}

static int CBaseCombatWeapon_DefaultTouch (lua_State *L) {
  luaL_checkweapon(L, 1)->DefaultTouch(luaL_checkentity(L, 2));
  return 0;
}

static int CBaseCombatWeapon_Deploy (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->Deploy());
  return 1;
}

static int CBaseCombatWeapon_DisplayAltFireHudHint (lua_State *L) {
  luaL_checkweapon(L, 1)->DisplayAltFireHudHint();
  return 0;
}

static int CBaseCombatWeapon_DisplayReloadHudHint (lua_State *L) {
  luaL_checkweapon(L, 1)->DisplayReloadHudHint();
  return 0;
}

static int CBaseCombatWeapon_Drop (lua_State *L) {
  luaL_checkweapon(L, 1)->Drop(luaL_checkvector(L, 2));
  return 0;
}

static int CBaseCombatWeapon_FinishReload (lua_State *L) {
  luaL_checkweapon(L, 1)->FinishReload();
  return 0;
}

static int CBaseCombatWeapon_GetActivity (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetActivity());
  return 1;
}

static int CBaseCombatWeapon_GetAnimPrefix (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetAnimPrefix());
  return 1;
}

static int CBaseCombatWeapon_GetBulletType (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetBulletType());
  return 1;
}

static int CBaseCombatWeapon_GetDamage (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetDamage(luaL_checknumber(L, 2), luaL_checkint(L, 3)));
  return 1;
}

static int CBaseCombatWeapon_GetDeathNoticeName (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetDeathNoticeName());
  return 1;
}

static int CBaseCombatWeapon_GetDefaultAnimSpeed (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetDefaultAnimSpeed());
  return 1;
}

static int CBaseCombatWeapon_GetDefaultClip1 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetDefaultClip1());
  return 1;
}

static int CBaseCombatWeapon_GetDefaultClip2 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetDefaultClip2());
  return 1;
}

static int CBaseCombatWeapon_GetDrawActivity (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetDrawActivity());
  return 1;
}

static int CBaseCombatWeapon_GetFireRate (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetFireRate());
  return 1;
}

static int CBaseCombatWeapon_GetIdealActivity (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetIdealActivity());
  return 1;
}

static int CBaseCombatWeapon_GetIdealSequence (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetIdealSequence());
  return 1;
}

static int CBaseCombatWeapon_GetMaxAutoAimDeflection (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetMaxAutoAimDeflection());
  return 1;
}

static int CBaseCombatWeapon_GetMaxBurst (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetMaxBurst());
  return 1;
}

static int CBaseCombatWeapon_GetMaxClip1 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetMaxClip1());
  return 1;
}

static int CBaseCombatWeapon_GetMaxClip2 (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetMaxClip2());
  return 1;
}

static int CBaseCombatWeapon_GetMaxRestTime (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetMaxRestTime());
  return 1;
}

static int CBaseCombatWeapon_GetMinBurst (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetMinBurst());
  return 1;
}

static int CBaseCombatWeapon_GetMinRestTime (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetMinRestTime());
  return 1;
}

static int CBaseCombatWeapon_GetName (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetName());
  return 1;
}

// FIXME: push CBaseCombatCharacter instead
static int CBaseCombatWeapon_GetOwner (lua_State *L) {
  lua_pushplayer(L, (CBasePlayer *)luaL_checkweapon(L, 1)->GetOwner());
  return 1;
}

static int CBaseCombatWeapon_GetPosition (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetPosition());
  return 1;
}

static int CBaseCombatWeapon_GetPrimaryAmmoCount (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetPrimaryAmmoCount());
  return 1;
}

static int CBaseCombatWeapon_GetPrimaryAmmoType (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetPrimaryAmmoType());
  return 1;
}

static int CBaseCombatWeapon_GetPrimaryAttackActivity (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetPrimaryAttackActivity());
  return 1;
}

static int CBaseCombatWeapon_GetPrintName (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetPrintName());
  return 1;
}

static int CBaseCombatWeapon_GetRandomBurst (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetRandomBurst());
  return 1;
}

static int CBaseCombatWeapon_GetRumbleEffect (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetRumbleEffect());
  return 1;
}

static int CBaseCombatWeapon_GetSecondaryAmmoCount (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetSecondaryAmmoCount());
  return 1;
}

static int CBaseCombatWeapon_GetSecondaryAmmoType (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetSecondaryAmmoType());
  return 1;
}

static int CBaseCombatWeapon_GetSecondaryAttackActivity (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetSecondaryAttackActivity());
  return 1;
}

static int CBaseCombatWeapon_GetShootSound (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetShootSound(luaL_checkint(L, 2)));
  return 1;
}

static int CBaseCombatWeapon_GetSlot (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetSlot());
  return 1;
}

static int CBaseCombatWeapon_GetSubType (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetSubType());
  return 1;
}

static int CBaseCombatWeapon_GetViewModel (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetViewModel(luaL_optint(L, 2, 0)));
  return 1;
}

static int CBaseCombatWeapon_GetViewModelSequenceDuration (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetViewModelSequenceDuration());
  return 1;
}

static int CBaseCombatWeapon_GetWeaponFlags (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetWeaponFlags());
  return 1;
}

static int CBaseCombatWeapon_GetWeaponIdleTime (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->GetWeaponIdleTime());
  return 1;
}

static int CBaseCombatWeapon_GetWeight (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->GetWeight());
  return 1;
}

static int CBaseCombatWeapon_GetWorldModel (lua_State *L) {
  lua_pushstring(L, luaL_checkweapon(L, 1)->GetWorldModel());
  return 1;
}

extern const char *pWeaponSoundCategories[ NUM_SHOOT_SOUND_TYPES ];

static int CBaseCombatWeapon_GetWpnData (lua_State *L) {
  const FileWeaponInfo_t &weaponInfo = luaL_checkweapon(L, 1)->GetWpnData();
  lua_newtable(L);
  lua_pushstring(L, "aShootSounds");
  lua_newtable(L);
  for ( int i = EMPTY; i < NUM_SHOOT_SOUND_TYPES; i++ )
  {
	lua_pushstring( L, pWeaponSoundCategories[i] );
	const char *soundname = weaponInfo.aShootSounds[i];
	if ( soundname && soundname[0] )
	{
		lua_pushstring( L, soundname );
		lua_settable( L, -3 );
	}
	else
	{
		lua_pop( L, 1 );
	}
  }
  lua_settable(L, -3);
  lua_pushstring(L, "bAutoSwitchFrom");
  lua_pushboolean(L, weaponInfo.bAutoSwitchFrom);
  lua_settable(L, -3);
  lua_pushstring(L, "bAutoSwitchTo");
  lua_pushboolean(L, weaponInfo.bAutoSwitchTo);
  lua_settable(L, -3);
  lua_pushstring(L, "bLoadedHudElements");
  lua_pushboolean(L, weaponInfo.bLoadedHudElements);
  lua_settable(L, -3);
  lua_pushstring(L, "bParsedScript");
  lua_pushboolean(L, weaponInfo.bParsedScript);
  lua_settable(L, -3);
  lua_pushstring(L, "bShowUsageHint");
  lua_pushboolean(L, weaponInfo.bShowUsageHint);
  lua_settable(L, -3);
  lua_pushstring(L, "iAmmo2Type");
  lua_pushinteger(L, weaponInfo.iAmmo2Type);
  lua_settable(L, -3);
  lua_pushstring(L, "iAmmoType");
  lua_pushinteger(L, weaponInfo.iAmmoType);
  lua_settable(L, -3);
  lua_pushstring(L, "iDefaultClip1");
  lua_pushinteger(L, weaponInfo.iDefaultClip1);
  lua_settable(L, -3);
  lua_pushstring(L, "iDefaultClip2");
  lua_pushinteger(L, weaponInfo.iDefaultClip2);
  lua_settable(L, -3);
  lua_pushstring(L, "iFlags");
  lua_pushinteger(L, weaponInfo.iFlags);
  lua_settable(L, -3);
  lua_pushstring(L, "iMaxClip1");
  lua_pushinteger(L, weaponInfo.iMaxClip1);
  lua_settable(L, -3);
  lua_pushstring(L, "iMaxClip2");
  lua_pushinteger(L, weaponInfo.iMaxClip2);
  lua_settable(L, -3);
  lua_pushstring(L, "iPosition");
  lua_pushinteger(L, weaponInfo.iPosition);
  lua_settable(L, -3);
  lua_pushstring(L, "iRumbleEffect");
  lua_pushinteger(L, weaponInfo.iRumbleEffect);
  lua_settable(L, -3);
  lua_pushstring(L, "iSlot");
  lua_pushinteger(L, weaponInfo.iSlot);
  lua_settable(L, -3);
  lua_pushstring(L, "iSpriteCount");
  lua_pushinteger(L, weaponInfo.iSpriteCount);
  lua_settable(L, -3);
  lua_pushstring(L, "iWeight");
  lua_pushinteger(L, weaponInfo.iWeight);
  lua_settable(L, -3);
  lua_pushstring(L, "m_bAllowFlipping");
  lua_pushboolean(L, weaponInfo.m_bAllowFlipping);
  lua_settable(L, -3);
  lua_pushstring(L, "m_bBuiltRightHanded");
  lua_pushboolean(L, weaponInfo.m_bBuiltRightHanded);
  lua_settable(L, -3);
  lua_pushstring(L, "m_bMeleeWeapon");
  lua_pushboolean(L, weaponInfo.m_bMeleeWeapon);
  lua_settable(L, -3);
  lua_pushstring(L, "szAmmo1");
  lua_pushstring(L, weaponInfo.szAmmo1);
  lua_settable(L, -3);
  lua_pushstring(L, "szAmmo2");
  lua_pushstring(L, weaponInfo.szAmmo2);
  lua_settable(L, -3);
  lua_pushstring(L, "szAnimationPrefix");
  lua_pushstring(L, weaponInfo.szAnimationPrefix);
  lua_settable(L, -3);
  lua_pushstring(L, "szClassName");
  lua_pushstring(L, weaponInfo.szClassName);
  lua_settable(L, -3);
  lua_pushstring(L, "szPrintName");
  lua_pushstring(L, weaponInfo.szPrintName);
  lua_settable(L, -3);
  lua_pushstring(L, "szViewModel");
  lua_pushstring(L, weaponInfo.szViewModel);
  lua_settable(L, -3);
  lua_pushstring(L, "szWorldModel");
  lua_pushstring(L, weaponInfo.szWorldModel);
  lua_settable(L, -3);
  return 1;
}

static int CBaseCombatWeapon_GiveDefaultAmmo (lua_State *L) {
  luaL_checkweapon(L, 1)->GiveDefaultAmmo();
  return 0;
}

static int CBaseCombatWeapon_HandleFireOnEmpty (lua_State *L) {
  luaL_checkweapon(L, 1)->HandleFireOnEmpty();
  return 0;
}

static int CBaseCombatWeapon_HasAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->HasAmmo());
  return 1;
}

static int CBaseCombatWeapon_HasAnyAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->HasAnyAmmo());
  return 1;
}

static int CBaseCombatWeapon_HasPrimaryAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->HasPrimaryAmmo());
  return 1;
}

static int CBaseCombatWeapon_HasSecondaryAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->HasSecondaryAmmo());
  return 1;
}

static int CBaseCombatWeapon_HasWeaponIdleTimeElapsed (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->HasWeaponIdleTimeElapsed());
  return 1;
}

static int CBaseCombatWeapon_HideThink (lua_State *L) {
  luaL_checkweapon(L, 1)->HideThink();
  return 0;
}

static int CBaseCombatWeapon_IsAllowedToSwitch (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsAllowedToSwitch());
  return 1;
}

static int CBaseCombatWeapon_IsLocked (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsLocked(luaL_checkentity(L, 2)));
  return 1;
}

static int CBaseCombatWeapon_IsMeleeWeapon (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsMeleeWeapon());
  return 1;
}

static int CBaseCombatWeapon_IsPredicted (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsPredicted());
  return 1;
}

static int CBaseCombatWeapon_IsScripted (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsScripted());
  return 1;
}

static int CBaseCombatWeapon_IsViewModelSequenceFinished (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsViewModelSequenceFinished());
  return 1;
}

static int CBaseCombatWeapon_IsWeaponVisible (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsWeaponVisible());
  return 1;
}

static int CBaseCombatWeapon_IsWeaponZoomed (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->IsWeaponZoomed());
  return 1;
}

static int CBaseCombatWeapon_ItemBusyFrame (lua_State *L) {
  luaL_checkweapon(L, 1)->ItemBusyFrame();
  return 0;
}

static int CBaseCombatWeapon_ItemHolsterFrame (lua_State *L) {
  luaL_checkweapon(L, 1)->ItemHolsterFrame();
  return 0;
}

static int CBaseCombatWeapon_ItemPostFrame (lua_State *L) {
  luaL_checkweapon(L, 1)->ItemPostFrame();
  return 0;
}

static int CBaseCombatWeapon_ItemPreFrame (lua_State *L) {
  luaL_checkweapon(L, 1)->ItemPreFrame();
  return 0;
}

static int CBaseCombatWeapon_Lock (lua_State *L) {
  luaL_checkweapon(L, 1)->Lock(luaL_checknumber(L, 2), luaL_checkentity(L, 3));
  return 0;
}

static int CBaseCombatWeapon_Lower (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->Lower());
  return 1;
}

static int CBaseCombatWeapon_MaintainIdealActivity (lua_State *L) {
  luaL_checkweapon(L, 1)->MaintainIdealActivity();
  return 0;
}

static int CBaseCombatWeapon_OnActiveStateChanged (lua_State *L) {
  luaL_checkweapon(L, 1)->OnActiveStateChanged(luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_OnRestore (lua_State *L) {
  luaL_checkweapon(L, 1)->OnRestore();
  return 0;
}

static int CBaseCombatWeapon_Precache (lua_State *L) {
  luaL_checkweapon(L, 1)->Precache();
  return 0;
}

static int CBaseCombatWeapon_PrimaryAttack (lua_State *L) {
  luaL_checkweapon(L, 1)->PrimaryAttack();
  return 0;
}

static int CBaseCombatWeapon_Ready (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->Ready());
  return 1;
}

static int CBaseCombatWeapon_Reload (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->Reload());
  return 1;
}

static int CBaseCombatWeapon_ReloadOrSwitchWeapons (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->ReloadOrSwitchWeapons());
  return 1;
}

static int CBaseCombatWeapon_RescindAltFireHudHint (lua_State *L) {
  luaL_checkweapon(L, 1)->RescindAltFireHudHint();
  return 0;
}

static int CBaseCombatWeapon_RescindReloadHudHint (lua_State *L) {
  luaL_checkweapon(L, 1)->RescindReloadHudHint();
  return 0;
}

static int CBaseCombatWeapon_SecondaryAttack (lua_State *L) {
  luaL_checkweapon(L, 1)->SecondaryAttack();
  return 0;
}

static int CBaseCombatWeapon_SendViewModelAnim (lua_State *L) {
  luaL_checkweapon(L, 1)->SendViewModelAnim(luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SendWeaponAnim (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->SendWeaponAnim(luaL_checkint(L, 2)));
  return 1;
}

static int CBaseCombatWeapon_SetActivity (lua_State *L) {
  luaL_checkweapon(L, 1)->SetActivity((Activity)luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SetIdealActivity (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->SetIdealActivity((Activity)luaL_checkint(L, 2)));
  return 1;
}

static int CBaseCombatWeapon_SetPickupTouch (lua_State *L) {
  luaL_checkweapon(L, 1)->SetPickupTouch();
  return 0;
}

static int CBaseCombatWeapon_SetPrimaryAmmoCount (lua_State *L) {
  luaL_checkweapon(L, 1)->SetPrimaryAmmoCount(luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SetSecondaryAmmoCount (lua_State *L) {
  luaL_checkweapon(L, 1)->SetSecondaryAmmoCount(luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SetSubType (lua_State *L) {
  luaL_checkweapon(L, 1)->SetSubType(luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SetViewModel (lua_State *L) {
  luaL_checkweapon(L, 1)->SetViewModel();
  return 0;
}

static int CBaseCombatWeapon_SetViewModelIndex (lua_State *L) {
  luaL_checkweapon(L, 1)->SetViewModelIndex(luaL_optint(L, 2, 0));
  return 0;
}

static int CBaseCombatWeapon_SetWeaponIdleTime (lua_State *L) {
  luaL_checkweapon(L, 1)->SetWeaponIdleTime(luaL_checknumber(L, 2));
  return 0;
}

static int CBaseCombatWeapon_SetWeaponVisible (lua_State *L) {
  luaL_checkweapon(L, 1)->SetWeaponVisible(luaL_checkboolean(L, 2));
  return 0;
}

static int CBaseCombatWeapon_ShouldDisplayAltFireHUDHint (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->ShouldDisplayAltFireHUDHint());
  return 1;
}

static int CBaseCombatWeapon_ShouldDisplayReloadHUDHint (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->ShouldDisplayReloadHUDHint());
  return 1;
}

static int CBaseCombatWeapon_ShouldShowControlPanels (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->ShouldShowControlPanels());
  return 1;
}

static int CBaseCombatWeapon_Spawn (lua_State *L) {
  luaL_checkweapon(L, 1)->Spawn();
  return 0;
}

static int CBaseCombatWeapon_StartSprinting (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->StartSprinting());
  return 1;
}

static int CBaseCombatWeapon_StopSprinting (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->StopSprinting());
  return 1;
}

static int CBaseCombatWeapon_StopWeaponSound (lua_State *L) {
  luaL_checkweapon(L, 1)->StopWeaponSound((WeaponSound_t)luaL_checkint(L, 2));
  return 0;
}

static int CBaseCombatWeapon_UsesClipsForAmmo1 (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->UsesClipsForAmmo1());
  return 1;
}

static int CBaseCombatWeapon_UsesClipsForAmmo2 (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->UsesClipsForAmmo2());
  return 1;
}

static int CBaseCombatWeapon_UsesPrimaryAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->UsesPrimaryAmmo());
  return 1;
}

static int CBaseCombatWeapon_UsesSecondaryAmmo (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->UsesSecondaryAmmo());
  return 1;
}

static int CBaseCombatWeapon_VisibleInWeaponSelection (lua_State *L) {
  lua_pushboolean(L, luaL_checkweapon(L, 1)->VisibleInWeaponSelection());
  return 1;
}

static int CBaseCombatWeapon_WeaponAutoAimScale (lua_State *L) {
  lua_pushnumber(L, luaL_checkweapon(L, 1)->WeaponAutoAimScale());
  return 1;
}

static int CBaseCombatWeapon_WeaponIdle (lua_State *L) {
  luaL_checkweapon(L, 1)->WeaponIdle();
  return 0;
}

static int CBaseCombatWeapon_WeaponSound (lua_State *L) {
  luaL_checkweapon(L, 1)->WeaponSound((WeaponSound_t)luaL_checkint(L, 2), luaL_optnumber(L, 2, 0.0f));
  return 0;
}

static int CBaseCombatWeapon_WeaponState (lua_State *L) {
  lua_pushinteger(L, luaL_checkweapon(L, 1)->WeaponState());
  return 1;
}

static int CBaseCombatWeapon___index (lua_State *L) {
  CBaseCombatWeapon *pWeapon = lua_toweapon(L, 1);
  if (pWeapon == NULL) {  /* avoid extra test when d is not 0 */
    lua_Debug ar1;
    lua_getstack(L, 1, &ar1);
    lua_getinfo(L, "fl", &ar1);
    lua_Debug ar2;
    lua_getinfo(L, ">S", &ar2);
	lua_pushfstring(L, "%s:%d: attempt to index a NULL entity", ar2.short_src, ar1.currentline);
	return lua_error(L);
  }
  const char *field = luaL_checkstring(L, 2);
  if (Q_strcmp(field, "m_bAltFiresUnderwater") == 0)
    lua_pushboolean(L, pWeapon->m_bAltFiresUnderwater);
  else if (Q_strcmp(field, "m_bFireOnEmpty") == 0)
    lua_pushboolean(L, pWeapon->m_bFireOnEmpty);
  else if (Q_strcmp(field, "m_bFiresUnderwater") == 0)
    lua_pushboolean(L, pWeapon->m_bFiresUnderwater);
  else if (Q_strcmp(field, "m_bInReload") == 0)
    lua_pushboolean(L, pWeapon->m_bInReload);
  else if (Q_strcmp(field, "m_bReloadsSingly") == 0)
    lua_pushboolean(L, pWeapon->m_bReloadsSingly);
  else if (Q_strcmp(field, "m_fFireDuration") == 0)
    lua_pushnumber(L, pWeapon->m_fFireDuration);
  else if (Q_strcmp(field, "m_flNextEmptySoundTime") == 0)
    lua_pushnumber(L, pWeapon->m_flNextEmptySoundTime);
  else if (Q_strcmp(field, "m_flNextPrimaryAttack") == 0)
    lua_pushnumber(L, pWeapon->m_flNextPrimaryAttack);
  else if (Q_strcmp(field, "m_flNextSecondaryAttack") == 0)
    lua_pushnumber(L, pWeapon->m_flNextSecondaryAttack);
  else if (Q_strcmp(field, "m_flTimeWeaponIdle") == 0)
    lua_pushnumber(L, pWeapon->m_flTimeWeaponIdle);
  else if (Q_strcmp(field, "m_flUnlockTime") == 0)
    lua_pushnumber(L, pWeapon->m_flUnlockTime);
  else if (Q_strcmp(field, "m_fMaxRange1") == 0)
    lua_pushnumber(L, pWeapon->m_fMaxRange1);
  else if (Q_strcmp(field, "m_fMaxRange2") == 0)
    lua_pushnumber(L, pWeapon->m_fMaxRange2);
  else if (Q_strcmp(field, "m_fMinRange1") == 0)
    lua_pushnumber(L, pWeapon->m_fMinRange1);
  else if (Q_strcmp(field, "m_fMinRange2") == 0)
    lua_pushnumber(L, pWeapon->m_fMinRange2);
  else if (Q_strcmp(field, "m_iClip1") == 0)
    lua_pushinteger(L, pWeapon->m_iClip1);
  else if (Q_strcmp(field, "m_iClip2") == 0)
    lua_pushinteger(L, pWeapon->m_iClip2);
  else if (Q_strcmp(field, "m_iPrimaryAmmoType") == 0)
    lua_pushinteger(L, pWeapon->m_iPrimaryAmmoType);
  else if (Q_strcmp(field, "m_iSecondaryAmmoType") == 0)
    lua_pushinteger(L, pWeapon->m_iSecondaryAmmoType);
  else if (Q_strcmp(field, "m_iState") == 0)
    lua_pushinteger(L, pWeapon->m_iState);
  else if (Q_strcmp(field, "m_iSubType") == 0)
    lua_pushinteger(L, pWeapon->m_iSubType);
  else if (Q_strcmp(field, "m_iViewModelIndex") == 0)
    lua_pushinteger(L, pWeapon->m_iViewModelIndex);
  else if (Q_strcmp(field, "m_iWorldModelIndex") == 0)
    lua_pushinteger(L, pWeapon->m_iWorldModelIndex);
  else if (Q_strcmp(field, "m_nViewModelIndex") == 0)
    lua_pushinteger(L, pWeapon->m_nViewModelIndex);
  else if (pWeapon->m_nTableReference != LUA_NOREF) {
    lua_getref(L, pWeapon->m_nTableReference);
    lua_getfield(L, -1, field);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 2);
      lua_getmetatable(L, 1);
      lua_getfield(L, -1, field);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        luaL_getmetatable(L, "CBaseAnimating");
        lua_getfield(L, -1, field);
        if (lua_isnil(L, -1)) {
          lua_pop(L, 2);
          luaL_getmetatable(L, "CBaseEntity");
          lua_getfield(L, -1, field);
        }
      }
    }
  }
  else {
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, field);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 2);
      luaL_getmetatable(L, "CBaseAnimating");
      lua_getfield(L, -1, field);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        luaL_getmetatable(L, "CBaseEntity");
        lua_getfield(L, -1, field);
      }
    }
  }
  return 1;
}

static int CBaseCombatWeapon___newindex (lua_State *L) {
  CBaseCombatWeapon *pWeapon = lua_toweapon(L, 1);
  if (pWeapon == NULL) {  /* avoid extra test when d is not 0 */
    lua_Debug ar1;
    lua_getstack(L, 1, &ar1);
    lua_getinfo(L, "fl", &ar1);
    lua_Debug ar2;
    lua_getinfo(L, ">S", &ar2);
	lua_pushfstring(L, "%s:%d: attempt to index a NULL entity", ar2.short_src, ar1.currentline);
	return lua_error(L);
  }
  const char *field = luaL_checkstring(L, 2);
  if (Q_strcmp(field, "m_bAltFiresUnderwater") == 0)
    pWeapon->m_bAltFiresUnderwater = luaL_checkboolean(L, 3);
  else if (Q_strcmp(field, "m_bFireOnEmpty") == 0)
    pWeapon->m_bFireOnEmpty = luaL_checkboolean(L, 3);
  else if (Q_strcmp(field, "m_bFiresUnderwater") == 0)
    pWeapon->m_bFiresUnderwater = luaL_checkboolean(L, 3);
  else if (Q_strcmp(field, "m_bInReload") == 0)
    pWeapon->m_bInReload = luaL_checkboolean(L, 3);
  else if (Q_strcmp(field, "m_bReloadsSingly") == 0)
    pWeapon->m_bReloadsSingly = luaL_checkboolean(L, 3);
  else if (Q_strcmp(field, "m_fFireDuration") == 0)
    pWeapon->m_fFireDuration = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_flNextEmptySoundTime") == 0)
    pWeapon->m_flNextEmptySoundTime = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_flNextPrimaryAttack") == 0)
    pWeapon->m_flNextPrimaryAttack.GetForModify() = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_flNextSecondaryAttack") == 0)
    pWeapon->m_flNextSecondaryAttack.GetForModify() = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_flTimeWeaponIdle") == 0)
    pWeapon->m_flTimeWeaponIdle.GetForModify() = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_flUnlockTime") == 0)
    pWeapon->m_flUnlockTime = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_fMaxRange1") == 0)
    pWeapon->m_fMaxRange1 = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_fMaxRange2") == 0)
    pWeapon->m_fMaxRange2 = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_fMinRange1") == 0)
    pWeapon->m_fMinRange1 = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_fMinRange2") == 0)
    pWeapon->m_fMinRange2 = luaL_checknumber(L, 3);
  else if (Q_strcmp(field, "m_iClip1") == 0)
    pWeapon->m_iClip1.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iClip2") == 0)
    pWeapon->m_iClip2.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iPrimaryAmmoType") == 0)
    pWeapon->m_iPrimaryAmmoType.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iSecondaryAmmoType") == 0)
    pWeapon->m_iSecondaryAmmoType.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iState") == 0)
    pWeapon->m_iState.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iSubType") == 0)
    pWeapon->m_iSubType = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iViewModelIndex") == 0)
    pWeapon->m_iViewModelIndex.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_iWorldModelIndex") == 0)
    pWeapon->m_iWorldModelIndex.GetForModify() = luaL_checkint(L, 3);
  else if (Q_strcmp(field, "m_nViewModelIndex") == 0)
    pWeapon->m_nViewModelIndex.GetForModify() = luaL_checkint(L, 3);
  else {
    if (pWeapon->m_nTableReference == LUA_NOREF) {
      lua_newtable(L);
      pWeapon->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    lua_getref(L, pWeapon->m_nTableReference);
    lua_pushvalue(L, 3);
    lua_setfield(L, -2, field);
	lua_pop(L, 1);
  }
  return 0;
}

static int CBaseCombatWeapon___eq (lua_State *L) {
  lua_pushboolean(L, lua_toweapon(L, 1) == lua_toweapon(L, 2));
  return 1;
}

static int CBaseCombatWeapon___tostring (lua_State *L) {
  CBaseCombatWeapon *pWeapon = lua_toweapon(L, 1);
  if (pWeapon == NULL)
    lua_pushstring(L, "NULL");
  else
    lua_pushfstring(L, "CBaseCombatWeapon: %d %s", pWeapon->entindex(), pWeapon->GetClassname());
  return 1;
}


// Native sequence control for SCP Lua weapons. Animation parity is replicated
// by the existing viewmodel, so clients do not need a custom animation protocol.
static int CBaseCombatWeapon_PlayPlayerAnimation(lua_State *state)
{
    CBaseCombatWeapon *weapon = luaL_checkweapon(state, 1);
    const char *action = luaL_checkstring(state, 2);
    PLAYER_ANIM event;
    if (!V_stricmp(action, "fire")) event = PLAYER_ATTACK1;
    else if (!V_stricmp(action, "reload")) event = PLAYER_RELOAD;
    else return luaL_error(state, "expected fire or reload player animation");
#ifndef CLIENT_DLL
    CBasePlayer *owner = ToBasePlayer(weapon->GetOwner());
    if (owner && owner->IsAlive()) owner->SetAnimation(event);
#endif
    return 0;
}

static int CBaseCombatWeapon_PlayWeaponSound(lua_State *state)
{
    CBaseCombatWeapon *weapon = luaL_checkweapon(state, 1);
    const char *path = luaL_checkstring(state, 2);
    const bool shot = lua_toboolean(state, 3) != 0;
    const int length = V_strlen(path);
    if (length < 5 || length >= MAX_PATH || V_stricmp(path + length - 4, ".wav") ||
        path[0] == '/' || path[0] == '\\' || V_strstr(path, "..") || V_strchr(path, ':') || V_strchr(path, '\\'))
        return luaL_error(state, "expected a relative WAV path");
    for (const char *p = path; *p; ++p)
        if ((unsigned char)*p < 32) return luaL_error(state, "invalid weapon sound path");
#ifndef CLIENT_DLL
    CBasePlayer *owner = ToBasePlayer(weapon->GetOwner());
    if (owner && owner->IsAlive())
    {
        const Vector origin = owner->EyePosition();
        const soundlevel_t level = shot ? SNDLVL_GUNFIRE : SNDLVL_NORM;
        CPASAttenuationFilter filter(origin, level);
        // This SWEP is server-authoritative: the shooter does not predict a copy.
        filter.SetIgnorePredictionCull(true);
        filter.AddRecipient(owner);
        EmitSound_t sound;
        sound.m_pSoundName = path;
        sound.m_nChannel = shot ? CHAN_WEAPON : CHAN_AUTO;
        sound.m_SoundLevel = level;
        sound.m_pOrigin = &origin;
        CBaseEntity::EmitSound(filter, owner->entindex(), sound);
    }
#endif
    return 0;
}

static int CBaseCombatWeapon_MeleeAttack(lua_State *state)
{
    CBaseCombatWeapon *weapon = luaL_checkweapon(state, 1);
    const float damage = luaL_checknumber(state, 2);
    const float range = luaL_checknumber(state, 3);
    if (!IsFinite(damage) || damage <= 0 || damage > 10000 ||
        !IsFinite(range) || range <= 0 || range > 128)
        return luaL_error(state, "invalid melee damage or range");
    bool hit = false;
#ifndef CLIENT_DLL
    CBasePlayer *owner = ToBasePlayer(weapon->GetOwner());
    if (owner && owner->IsAlive())
    {
        const Vector start = owner->Weapon_ShootPosition();
        const Vector direction = owner->GetAutoaimVector(0);
        const Vector end = start + direction * range;
        trace_t trace;
        UTIL_TraceLine(start, end, MASK_SHOT_HULL, owner, COLLISION_GROUP_NONE, &trace);
        if (trace.fraction == 1.0f)
            UTIL_TraceHull(start, end, Vector(-8,-8,-8), Vector(8,8,8), MASK_SHOT_HULL,
                owner, COLLISION_GROUP_NONE, &trace);
        hit = trace.fraction < 1.0f;
        if (hit && trace.m_pEnt)
        {
            CTakeDamageInfo info(weapon, owner, weapon, damage, DMG_CLUB);
            CalculateMeleeDamageForce(&info, direction, trace.endpos);
            ClearMultiDamage();
            trace.m_pEnt->DispatchTraceAttack(info, direction, &trace);
            ApplyMultiDamage();
            UTIL_ImpactTrace(&trace, DMG_CLUB);
        }
    }
#endif
    lua_pushboolean(state, hit);
    return 1;
}

static int CBaseCombatWeapon_SetAimState(lua_State *state)
{
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(luaL_checkweapon(state, 1));
#ifndef CLIENT_DLL
    if (weapon) weapon->SetAimState(lua_toboolean(state, 2) != 0);
#endif
    return 0;
}

static int CBaseCombatWeapon_IsAiming(lua_State *state)
{
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(luaL_checkweapon(state, 1));
    lua_pushboolean(state, weapon && weapon->IsAiming());
    return 1;
}

static int CBaseCombatWeapon_GetModuleSelection(lua_State *state)
{
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(luaL_checkweapon(state, 1));
    const int slot = (int)luaL_checkinteger(state, 2) - 1;
    lua_pushinteger(state, weapon ? weapon->GetSWEPModuleSelection(slot) + 1 : 1);
    return 1;
}

static int CBaseCombatWeapon_IsSafetyOn(lua_State *state)
{
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(luaL_checkweapon(state, 1));
    lua_pushboolean(state, weapon && weapon->IsSafetyOn());
    return 1;
}

static int CBaseCombatWeapon_SetSafetyState(lua_State *state)
{
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(luaL_checkweapon(state, 1));
    luaL_checktype(state, 2, LUA_TBOOLEAN);
#ifndef CLIENT_DLL
    CBasePlayer *owner = weapon ? ToBasePlayer(weapon->GetOwner()) : NULL;
    const bool enabled = lua_toboolean(state, 2) != 0;
    if (owner && owner->IsAlive() && owner->GetActiveWeapon() == weapon &&
        weapon->IsNativeSWEP() && weapon->IsSafetyOn() != enabled)
    {
        weapon->SetSafetyState(enabled);
    }
#endif
    return 0;
}

static int CBaseCombatWeapon_GetClassname(lua_State *state)
{
    CBaseCombatWeapon *weapon = luaL_checkweapon(state, 1);
#ifdef CLIENT_DLL
    CHL2MPScriptedWeapon *scripted = dynamic_cast<CHL2MPScriptedWeapon *>(weapon);
    if (scripted && scripted->GetScriptedClassname()[0])
    {
        lua_pushstring(state, scripted->GetScriptedClassname());
        return 1;
    }
#endif
    lua_pushstring(state, weapon->GetClassname());
    return 1;
}

static int CBaseCombatWeapon_PlaySequence(lua_State *state)
{
    CBaseCombatWeapon *weapon=luaL_checkweapon(state,1);
    CBasePlayer *owner=ToBasePlayer(weapon->GetOwner());
    CBaseViewModel *model=owner ? owner->GetViewModel() : NULL;
    if(!model) { lua_pushnumber(state,0); return 1; }
    int sequence=model->LookupSequence(luaL_checkstring(state,2));
    if(sequence<0) return luaL_error(state,"weapon animation not found: %s",lua_tostring(state,2));
    const float naturalDuration = MAX(0.01f, model->SequenceDuration(sequence));
    const float rate = luaL_optnumber(state, 4, 1.0f);
    if (!IsFinite(rate) || rate <= 0 || rate > 12)
        return luaL_error(state, "invalid weapon animation rate");
    const float duration = luaL_optnumber(state, 3, naturalDuration / rate);
    if (!IsFinite(duration) || duration <= 0 || duration > 120)
        return luaL_error(state, "invalid weapon animation duration");
    model->SendViewModelMatchingSequence(sequence);
    model->SetScriptedSequenceLoop(lua_toboolean(state, 5) != 0);
    model->SetPlaybackRate(naturalDuration / duration);
    weapon->SetWeaponIdleTime(gpGlobals->curtime + duration);
    lua_pushnumber(state,duration); return 1;
}
static int CBaseCombatWeapon_DeployModel(lua_State *state)
{
    CBaseCombatWeapon *weapon=luaL_checkweapon(state,1);
    lua_pushboolean(state,weapon->DefaultDeploy((char *)weapon->GetViewModel(),(char *)weapon->GetWorldModel(),ACT_VM_DRAW,(char *)weapon->GetAnimPrefix()));
    return 1;
}
static const luaL_Reg CBaseCombatWeaponmeta[] = {
  {"GetModuleSelection", CBaseCombatWeapon_GetModuleSelection},
  {"IsSafetyOn",CBaseCombatWeapon_IsSafetyOn},
  {"SetSafetyState",CBaseCombatWeapon_SetSafetyState},
  {"PlayWeaponSound",CBaseCombatWeapon_PlayWeaponSound},
  {"MeleeAttack",CBaseCombatWeapon_MeleeAttack},
  {"PlayPlayerAnimation",CBaseCombatWeapon_PlayPlayerAnimation},
  {"SetAimState",CBaseCombatWeapon_SetAimState},
  {"IsAiming",CBaseCombatWeapon_IsAiming},
  {"GetClassname",CBaseCombatWeapon_GetClassname},
  {"PlaySequence",CBaseCombatWeapon_PlaySequence},
  {"DeployModel",CBaseCombatWeapon_DeployModel},
  {"AbortReload", CBaseCombatWeapon_AbortReload},
  {"Activate", CBaseCombatWeapon_Activate},
  {"ActivityListCount", CBaseCombatWeapon_ActivityListCount},
  {"ActivityOverride", CBaseCombatWeapon_ActivityOverride},
  {"AddViewKick", CBaseCombatWeapon_AddViewKick},
  {"AllowsAutoSwitchFrom", CBaseCombatWeapon_AllowsAutoSwitchFrom},
  {"AllowsAutoSwitchTo", CBaseCombatWeapon_AllowsAutoSwitchTo},
  {"CalcViewmodelBob", CBaseCombatWeapon_CalcViewmodelBob},
  {"CanBePickedUpByNPCs", CBaseCombatWeapon_CanBePickedUpByNPCs},
  {"CanBeSelected", CBaseCombatWeapon_CanBeSelected},
  {"CanDeploy", CBaseCombatWeapon_CanDeploy},
  {"CanHolster", CBaseCombatWeapon_CanHolster},
  {"CanLower", CBaseCombatWeapon_CanLower},
  {"CheckReload", CBaseCombatWeapon_CheckReload},
  {"Clip1", CBaseCombatWeapon_Clip1},
  {"Clip2", CBaseCombatWeapon_Clip2},
  {"DefaultDeploy", CBaseCombatWeapon_DefaultDeploy},
  {"DefaultReload", CBaseCombatWeapon_DefaultReload},
  {"DefaultTouch", CBaseCombatWeapon_DefaultTouch},
  {"Deploy", CBaseCombatWeapon_Deploy},
  {"DisplayAltFireHudHint", CBaseCombatWeapon_DisplayAltFireHudHint},
  {"DisplayReloadHudHint", CBaseCombatWeapon_DisplayReloadHudHint},
  {"Drop", CBaseCombatWeapon_Drop},
  {"FinishReload", CBaseCombatWeapon_FinishReload},
  {"GetActivity", CBaseCombatWeapon_GetActivity},
  {"GetAnimPrefix", CBaseCombatWeapon_GetAnimPrefix},
  {"GetBulletType", CBaseCombatWeapon_GetBulletType},
  {"GetDamage", CBaseCombatWeapon_GetDamage},
  {"GetDeathNoticeName", CBaseCombatWeapon_GetDeathNoticeName},
  {"GetDefaultAnimSpeed", CBaseCombatWeapon_GetDefaultAnimSpeed},
  {"GetDefaultClip1", CBaseCombatWeapon_GetDefaultClip1},
  {"GetDefaultClip2", CBaseCombatWeapon_GetDefaultClip2},
  {"GetDrawActivity", CBaseCombatWeapon_GetDrawActivity},
  {"GetFireRate", CBaseCombatWeapon_GetFireRate},
  {"GetIdealActivity", CBaseCombatWeapon_GetIdealActivity},
  {"GetIdealSequence", CBaseCombatWeapon_GetIdealSequence},
  {"GetMaxAutoAimDeflection", CBaseCombatWeapon_GetMaxAutoAimDeflection},
  {"GetMaxBurst", CBaseCombatWeapon_GetMaxBurst},
  {"GetMaxClip1", CBaseCombatWeapon_GetMaxClip1},
  {"GetMaxClip2", CBaseCombatWeapon_GetMaxClip2},
  {"GetMaxRestTime", CBaseCombatWeapon_GetMaxRestTime},
  {"GetMinBurst", CBaseCombatWeapon_GetMinBurst},
  {"GetMinRestTime", CBaseCombatWeapon_GetMinRestTime},
  {"GetName", CBaseCombatWeapon_GetName},
  {"GetOwner", CBaseCombatWeapon_GetOwner},
  {"GetPosition", CBaseCombatWeapon_GetPosition},
  {"GetPrimaryAmmoCount", CBaseCombatWeapon_GetPrimaryAmmoCount},
  {"GetPrimaryAmmoType", CBaseCombatWeapon_GetPrimaryAmmoType},
  {"GetPrimaryAttackActivity", CBaseCombatWeapon_GetPrimaryAttackActivity},
  {"GetPrintName", CBaseCombatWeapon_GetPrintName},
  {"GetRandomBurst", CBaseCombatWeapon_GetRandomBurst},
  {"GetRumbleEffect", CBaseCombatWeapon_GetRumbleEffect},
  {"GetSecondaryAmmoCount", CBaseCombatWeapon_GetSecondaryAmmoCount},
  {"GetSecondaryAmmoType", CBaseCombatWeapon_GetSecondaryAmmoType},
  {"GetSecondaryAttackActivity", CBaseCombatWeapon_GetSecondaryAttackActivity},
  {"GetShootSound", CBaseCombatWeapon_GetShootSound},
  {"GetSlot", CBaseCombatWeapon_GetSlot},
  {"GetSubType", CBaseCombatWeapon_GetSubType},
  {"GetViewModel", CBaseCombatWeapon_GetViewModel},
  {"GetViewModelSequenceDuration", CBaseCombatWeapon_GetViewModelSequenceDuration},
  {"GetWeaponFlags", CBaseCombatWeapon_GetWeaponFlags},
  {"GetWeaponIdleTime", CBaseCombatWeapon_GetWeaponIdleTime},
  {"GetWeight", CBaseCombatWeapon_GetWeight},
  {"GetWorldModel", CBaseCombatWeapon_GetWorldModel},
  {"GetWpnData", CBaseCombatWeapon_GetWpnData},
  {"GiveDefaultAmmo", CBaseCombatWeapon_GiveDefaultAmmo},
  {"HandleFireOnEmpty", CBaseCombatWeapon_HandleFireOnEmpty},
  {"HasAmmo", CBaseCombatWeapon_HasAmmo},
  {"HasAmmo", CBaseCombatWeapon_HasAnyAmmo},
  {"HasPrimaryAmmo", CBaseCombatWeapon_HasPrimaryAmmo},
  {"HasSecondaryAmmo", CBaseCombatWeapon_HasSecondaryAmmo},
  {"HasWeaponIdleTimeElapsed", CBaseCombatWeapon_HasWeaponIdleTimeElapsed},
  {"HideThink", CBaseCombatWeapon_HideThink},
  {"IsAllowedToSwitch", CBaseCombatWeapon_IsAllowedToSwitch},
  {"IsLocked", CBaseCombatWeapon_IsLocked},
  {"IsMeleeWeapon", CBaseCombatWeapon_IsMeleeWeapon},
  {"IsPredicted", CBaseCombatWeapon_IsPredicted},
  {"IsScripted", CBaseCombatWeapon_IsScripted},
  {"IsViewModelSequenceFinished", CBaseCombatWeapon_IsViewModelSequenceFinished},
  {"IsWeaponVisible", CBaseCombatWeapon_IsWeaponVisible},
  {"IsWeaponZoomed", CBaseCombatWeapon_IsWeaponZoomed},
  {"ItemBusyFrame", CBaseCombatWeapon_ItemBusyFrame},
  {"ItemHolsterFrame", CBaseCombatWeapon_ItemHolsterFrame},
  {"ItemPostFrame", CBaseCombatWeapon_ItemPostFrame},
  {"ItemPreFrame", CBaseCombatWeapon_ItemPreFrame},
  {"Lock", CBaseCombatWeapon_Lock},
  {"Lower", CBaseCombatWeapon_Lower},
  {"MaintainIdealActivity", CBaseCombatWeapon_MaintainIdealActivity},
  {"OnActiveStateChanged", CBaseCombatWeapon_OnActiveStateChanged},
  {"OnRestore", CBaseCombatWeapon_OnRestore},
  {"Precache", CBaseCombatWeapon_Precache},
  {"PrimaryAttack", CBaseCombatWeapon_PrimaryAttack},
  {"Ready", CBaseCombatWeapon_Ready},
  {"Reload", CBaseCombatWeapon_Reload},
  {"ReloadOrSwitchWeapons", CBaseCombatWeapon_ReloadOrSwitchWeapons},
  {"RescindAltFireHudHint", CBaseCombatWeapon_RescindAltFireHudHint},
  {"RescindReloadHudHint", CBaseCombatWeapon_RescindReloadHudHint},
  {"SecondaryAttack", CBaseCombatWeapon_SecondaryAttack},
  {"SendViewModelAnim", CBaseCombatWeapon_SendViewModelAnim},
  {"SendWeaponAnim", CBaseCombatWeapon_SendWeaponAnim},
  {"SetActivity", CBaseCombatWeapon_SetActivity},
  {"SetIdealActivity", CBaseCombatWeapon_SetIdealActivity},
  {"SetPickupTouch", CBaseCombatWeapon_SetPickupTouch},
  {"SetPrimaryAmmoCount", CBaseCombatWeapon_SetPrimaryAmmoCount},
  {"SetSecondaryAmmoCount", CBaseCombatWeapon_SetSecondaryAmmoCount},
  {"SetSubType", CBaseCombatWeapon_SetSubType},
  {"SetViewModel", CBaseCombatWeapon_SetViewModel},
  {"SetViewModelIndex", CBaseCombatWeapon_SetViewModelIndex},
  {"SetWeaponIdleTime", CBaseCombatWeapon_SetWeaponIdleTime},
  {"SetWeaponVisible", CBaseCombatWeapon_SetWeaponVisible},
  {"ShouldDisplayAltFireHUDHint", CBaseCombatWeapon_ShouldDisplayAltFireHUDHint},
  {"ShouldDisplayReloadHUDHint", CBaseCombatWeapon_ShouldDisplayReloadHUDHint},
  {"ShouldShowControlPanels", CBaseCombatWeapon_ShouldShowControlPanels},
  {"Spawn", CBaseCombatWeapon_Spawn},
  {"StartSprinting", CBaseCombatWeapon_StartSprinting},
  {"StopSprinting", CBaseCombatWeapon_StopSprinting},
  {"StopWeaponSound", CBaseCombatWeapon_StopWeaponSound},
  {"UsesClipsForAmmo1", CBaseCombatWeapon_UsesClipsForAmmo1},
  {"UsesClipsForAmmo2", CBaseCombatWeapon_UsesClipsForAmmo2},
  {"UsesPrimaryAmmo", CBaseCombatWeapon_UsesPrimaryAmmo},
  {"UsesSecondaryAmmo", CBaseCombatWeapon_UsesSecondaryAmmo},
  {"VisibleInWeaponSelection", CBaseCombatWeapon_VisibleInWeaponSelection},
  {"WeaponAutoAimScale", CBaseCombatWeapon_WeaponAutoAimScale},
  {"WeaponIdle", CBaseCombatWeapon_WeaponIdle},
  {"WeaponSound", CBaseCombatWeapon_WeaponSound},
  {"WeaponState", CBaseCombatWeapon_WeaponState},
  {"__index", CBaseCombatWeapon___index},
  {"__newindex", CBaseCombatWeapon___newindex},
  {"__eq", CBaseCombatWeapon___eq},
  {"__tostring", CBaseCombatWeapon___tostring},
  {NULL, NULL}
};


/*
** Open CBaseCombatWeapon object
*/
LUALIB_API int luaopen_CBaseCombatWeapon (lua_State *L) {
  luaL_newmetatable(L, LUA_BASECOMBATWEAPONLIBNAME);
  luaL_register(L, NULL, CBaseCombatWeaponmeta);
  lua_pushstring(L, "entity");
  lua_setfield(L, -2, "__type");  /* metatable.__type = "entity" */
  return 1;
}

