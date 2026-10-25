//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#if defined( CLIENT_DLL )
	#include "hl2/c_basehlplayer.h"
#else
	#include "hl2/hl2_player.h"
#endif

#include "weapon_hl2mpbase_scriptedweapon.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "model_types.h"
#include "luamanager.h"
#include "lbasecombatweapon_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef LUA_SDK
// A native getter can run inside an existing Lua call (for example DeployModel).
// Isolate its temporary stack without discarding that caller's arguments.
class LuaWeaponFrame {
    lua_State *state; int saved,reference;
public:
    explicit LuaWeaponFrame(lua_State *vm) : state(vm),saved(vm ? lua_gettop(vm) : 0),reference(LUA_NOREF) {
        if(!saved) return;
        lua_createtable(state,saved,0);
        for(int i=1;i<=saved;++i) { lua_pushvalue(state,i); lua_rawseti(state,-2,i); }
        reference=luaL_ref(state,LUA_REGISTRYINDEX); lua_settop(state,0);
    }
    ~LuaWeaponFrame() {
        if(!state) return;
        lua_settop(state,0);
        if(reference==LUA_NOREF) return;
        lua_rawgeti(state,LUA_REGISTRYINDEX,reference);
        for(int i=1;i<=saved;++i) lua_rawgeti(state,1,i);
        lua_remove(state,1); luaL_unref(state,LUA_REGISTRYINDEX,reference);
    }
};
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPScriptedWeapon, DT_HL2MPScriptedWeapon )

BEGIN_NETWORK_TABLE( CHL2MPScriptedWeapon, DT_HL2MPScriptedWeapon )
#ifdef CLIENT_DLL
	RecvPropString( RECVINFO( m_iScriptedClassname ) ),
    RecvPropBool(RECVINFO(m_bSWEPAiming)),
    RecvPropBool(RECVINFO(m_bSWEPSafety)),
    RecvPropArray3(RECVINFO_ARRAY(m_iSWEPModules), RecvPropInt(RECVINFO(m_iSWEPModules[0]))),
#else
	SendPropString( SENDINFO( m_iScriptedClassname ) ),
    SendPropBool(SENDINFO(m_bSWEPAiming)),
    SendPropBool(SENDINFO(m_bSWEPSafety)),
    SendPropArray3(SENDINFO_ARRAY3(m_iSWEPModules), SendPropInt(SENDINFO_ARRAY(m_iSWEPModules), 5, SPROP_UNSIGNED)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CHL2MPScriptedWeapon )
END_PREDICTION_DATA()

//=========================================================
//	>> CHLSelectFireScriptedWeapon
//=========================================================
BEGIN_DATADESC( CHL2MPScriptedWeapon )
    DEFINE_FIELD(m_bSWEPSafety, FIELD_BOOLEAN),
    DEFINE_ARRAY(m_iSWEPModules, FIELD_INTEGER, SCP_WEAPON_MODULE_SLOTS),
END_DATADESC()



// LINK_ENTITY_TO_CLASS( weapon_hl2mpbase_scriptedweapon, CHL2MPScriptedWeapon );
// PRECACHE_WEAPON_REGISTER( weapon_hl2mpbase_scriptedweapon );

// These functions replace the macros above for runtime registration of
// scripted weapons.
#ifdef CLIENT_DLL
static C_BaseEntity *CCHL2MPScriptedWeaponFactory( void )
{
	return static_cast< C_BaseEntity * >( new CHL2MPScriptedWeapon );
};
#endif

#ifndef CLIENT_DLL
static CUtlDict< CEntityFactory<CHL2MPScriptedWeapon>*, unsigned short > m_WeaponFactoryDatabase;
#endif

void RegisterScriptedWeapon( const char *className )
{
#ifdef CLIENT_DLL
	if ( GetClassMap().FindFactory( className ) )
	{
		return;
	}

	GetClassMap().Add( className, "CHL2MPScriptedWeapon", sizeof( CHL2MPScriptedWeapon ),
		&CCHL2MPScriptedWeaponFactory, true );
#else
	if ( EntityFactoryDictionary()->FindFactory( className ) )
	{
		return;
	}

	unsigned short lookup = m_WeaponFactoryDatabase.Find( className );
	if ( lookup != m_WeaponFactoryDatabase.InvalidIndex() )
	{
		return;
	}

	// Andrew; This fixes months worth of pain and anguish.
	CEntityFactory<CHL2MPScriptedWeapon> *pFactory = new CEntityFactory<CHL2MPScriptedWeapon>( className );

	lookup = m_WeaponFactoryDatabase.Insert( className, pFactory );
	Assert( lookup != m_WeaponFactoryDatabase.InvalidIndex() );
#endif
	// BUGBUG: When attempting to precache weapons registered during runtime,
	// they don't appear as valid registered entities.
	// static CPrecacheRegister precache_weapon_(&CPrecacheRegister::PrecacheFn_Other, className);
}

void ResetWeaponFactoryDatabase( void )
{
#ifdef CLIENT_DLL
#ifdef LUA_SDK
	GetClassMap().RemoveAllScripted();
#endif
#else
	for ( int i=m_WeaponFactoryDatabase.First(); i != m_WeaponFactoryDatabase.InvalidIndex(); i=m_WeaponFactoryDatabase.Next( i ) )
	{
		delete m_WeaponFactoryDatabase[ i ];
	}
	m_WeaponFactoryDatabase.RemoveAll();
#endif
}


// acttable_t CHL2MPScriptedWeapon::m_acttable[] = 
// {
// 	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PISTOL,					false },
// 	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },
// 
// 	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PISTOL,					false },
// 	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PISTOL,			false },
// 
// 	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
// 	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
// 
// 	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
// 	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
// 
// 	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PISTOL,					false },
// };

// IMPLEMENT_ACTTABLE( CHL2MPScriptedWeapon );

// These functions serve as skeletons for the our weapons' actions to be
// implemented in Lua.
// NPC and multiplayer player models use different activity families. Resolve
// the authored hold type against sequences actually present in the owner's MDL.
// Legacy Lua weapons retain their explicit m_acttable behavior.
int CHL2MPScriptedWeapon::GetSWEPModuleCount() const
{
    if (!L || m_nTableReference < 0) return 0;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "Modules");
    return lua_istable(L, -1) ? MIN((int)lua_objlen(L, -1), SCP_WEAPON_MODULE_SLOTS) : 0;
}

int CHL2MPScriptedWeapon::GetSWEPModuleOptionCount(int slot) const
{
    if (slot < 0 || slot >= GetSWEPModuleCount()) return 0;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "Modules"); lua_rawgeti(L, -1, slot + 1);
    lua_getfield(L, -1, "Options");
    return lua_istable(L, -1) ? MIN((int)lua_objlen(L, -1), SCP_WEAPON_MODULE_OPTIONS) : 0;
}

void CHL2MPScriptedWeapon::GetSWEPModuleName(int slot, char *buffer, int size) const
{
    if (size <= 0) return;
    buffer[0] = 0;
    if (slot < 0 || slot >= GetSWEPModuleCount()) return;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "Modules"); lua_rawgeti(L, -1, slot + 1);
    lua_getfield(L, -1, "Name");
    if (lua_isstring(L, -1)) Q_strncpy(buffer, lua_tostring(L, -1), size);
}

static void ReadSWEPModuleMount(lua_State *state, int table, const char *field, SCPWeaponModuleMount &out)
{
    lua_getfield(state, table, field);
    if (lua_istable(state, -1))
    {
        const int mount = lua_gettop(state);
        lua_getfield(state, mount, "Bone");
        if (lua_isstring(state, -1)) Q_strncpy(out.bone, lua_tostring(state, -1), sizeof(out.bone));
        lua_pop(state, 1);
        lua_getfield(state, mount, "Bonemerge");
        if (lua_isboolean(state, -1)) out.bonemerge = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        lua_getfield(state, mount, "RelativeToModule");
        out.relativeToModule = lua_toboolean(state, -1) != 0;
        lua_pop(state, 1);
        const char *fields[] = { "Position", "Angles" };
        const char *axes[] = { "x", "y", "z" };
        for (int fieldIndex = 0; fieldIndex < 2; ++fieldIndex)
        {
            lua_getfield(state, mount, fields[fieldIndex]);
            if (lua_istable(state, -1) || lua_isuserdata(state, -1))
            {
                Vector &value = fieldIndex ? out.angles : out.position;
                for (int axis = 0; axis < 3; ++axis)
                {
                    lua_getfield(state, -1, axes[axis]);
                    if (lua_isnumber(state, -1)) value[axis] = clamp((float)lua_tonumber(state, -1), -360.0f, 360.0f);
                    lua_pop(state, 1);
                }
            }
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
}

bool CHL2MPScriptedWeapon::GetSWEPModuleOption(int slot, int option, SCPWeaponModuleOption &out) const
{
    if (option < 0 || option >= GetSWEPModuleOptionCount(slot)) return false;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "Modules"); lua_rawgeti(L, -1, slot + 1);
    lua_getfield(L, -1, "Options"); lua_rawgeti(L, -1, option + 1);
    const int table = lua_gettop(L);
    out = SCPWeaponModuleOption();
    const char *keys[] = { "Name", "ViewModel", "WorldModel", "Reticle" };
    char *targets[] = { out.name, out.viewModel, out.worldModel, out.reticle };
    const int sizes[] = { sizeof(out.name), sizeof(out.viewModel), sizeof(out.worldModel), sizeof(out.reticle) };
    for (int i = 0; i < 4; ++i)
    {
        lua_getfield(L, table, keys[i]);
        if (lua_isstring(L, -1)) Q_strncpy(targets[i], lua_tostring(L, -1), sizes[i]);
        lua_pop(L, 1);
    }
    lua_getfield(L, table, "Scale");
    if (lua_isnumber(L, -1)) out.scale = clamp((float)lua_tonumber(L, -1), 0.1f, 4.0f);
    lua_pop(L, 1);
    lua_getfield(L, table, "ScopeFOV");
    if (lua_isnumber(L, -1)) out.scopeFOV = clamp((float)lua_tonumber(L, -1), 1.0f, 89.0f);
    lua_pop(L, 1);
    lua_getfield(L, table, "ReticleSize");
    if (lua_isnumber(L, -1)) out.reticleSize = clamp((float)lua_tonumber(L, -1), 4.0f, 512.0f);
    lua_pop(L, 1);
    ReadSWEPModuleMount(L, table, "ViewMount", out.viewMount);
    ReadSWEPModuleMount(L, table, "WorldMount", out.worldMount);
    lua_getfield(L, table, "Lens");
    if (lua_istable(L, -1))
    {
        const int lens = lua_gettop(L);
        lua_getfield(L, lens, "Model");
        if (lua_isstring(L, -1)) Q_strncpy(out.lensModel, lua_tostring(L, -1), sizeof(out.lensModel));
        lua_pop(L, 1);
        lua_getfield(L, lens, "Scale");
        if (lua_isnumber(L, -1)) out.lensScale = clamp((float)lua_tonumber(L, -1), 0.1f, 4.0f);
        lua_pop(L, 1);
        ReadSWEPModuleMount(L, lens, "Mount", out.lensMount);
    }
    lua_pop(L, 1);
    const char *groups[] = { "Bodygroups", "WorldBodygroups" };
    for (int i = 0; i < 2; ++i)
    {
        lua_getfield(L, table, groups[i]);
        if (lua_istable(L, -1))
        {
            lua_pushnil(L);
            if (lua_next(L, -2))
            {
                int &group = i ? out.worldBodygroup : out.bodygroup;
                int &value = i ? out.worldBodyValue : out.bodyValue;
                group = (int)lua_tointeger(L, -2); value = (int)lua_tointeger(L, -1);
                lua_pop(L, 2);
            }
        }
        lua_pop(L, 1);
    }
    return true;
}

void CHL2MPScriptedWeapon::ApplySWEPModules(CBaseAnimating *target, bool world) const
{
    if (!target || GetSWEPModuleCount() == 0) return;
    for (int group = 0; group < target->GetNumBodyGroups(); ++group) target->SetBodygroup(group, 0);
    for (int slot = 0; slot < GetSWEPModuleCount(); ++slot)
    {
        SCPWeaponModuleOption option;
        if (!GetSWEPModuleOption(slot, GetSWEPModuleSelection(slot), option)) continue;
        const int group = world ? option.worldBodygroup : option.bodygroup;
        const int value = world ? option.worldBodyValue : option.bodyValue;
        if (group >= 0 && group < target->GetNumBodyGroups() &&
            value >= 0 && value < target->GetBodygroupCount(group)) target->SetBodygroup(group, value);
    }
}

#ifndef CLIENT_DLL
bool CHL2MPScriptedWeapon::SetSWEPModule(int slot, int option)
{
    CBasePlayer *owner = ToBasePlayer(GetOwner());
    if (!owner || !owner->IsAlive() || owner->GetActiveWeapon() != this ||
        m_flNextPrimaryAttack > gpGlobals->curtime || slot < 0 || slot >= GetSWEPModuleCount() ||
        option < 0 || option >= GetSWEPModuleOptionCount(slot)) return false;
    m_iSWEPModules.Set(slot, option);
    ApplySWEPModules(this, true);
    if (owner->GetViewModel()) ApplySWEPModules(owner->GetViewModel(), false);
    m_flNextPrimaryAttack = MAX(m_flNextPrimaryAttack, gpGlobals->curtime + 0.15f);
    return true;
}
#else
class C_SCPWeaponModulePart : public C_BaseAnimating
{
public:
    bool view = false;
    bool IsViewModel() const OVERRIDE { return view; }
    bool ShouldDraw() OVERRIDE { return false; }
    ShadowType_t ShadowCastType() OVERRIDE { return SHADOWS_NONE; }
};

static void RotateSWEPModuleMount(QAngle &angles, const Vector &axis, float degrees)
{
    Quaternion turn;
    AxisAngleQuaternion(axis, degrees, turn);
    matrix3x4_t rotation, initial, result;
    QuaternionMatrix(turn, rotation);
    AngleMatrix(angles, initial);
    ConcatTransforms(rotation, initial, result);
    MatrixAngles(result, angles);
}

static void DrawSWEPModuleModel(CHandle<C_BaseAnimating> &handle, C_BaseAnimating *parent,
    const char *path, const SCPWeaponModuleMount &mount, float scale, int flags, bool world)
{
    const int index = path[0] ? modelinfo->GetModelIndex(path) : -1;
    if (handle && (handle->GetModelIndex() != index ||
        ((handle->IsEffectActive(EF_BONEMERGE) != 0) != mount.bonemerge))) handle->Release();
    if (index <= 0) return;
    if (!handle)
    {
        C_SCPWeaponModulePart *part = new C_SCPWeaponModulePart;
        part->view = !world;
        if (!part->InitializeAsClientEntity(path, world ? RENDER_GROUP_OPAQUE_ENTITY : RENDER_GROUP_VIEW_MODEL_OPAQUE))
        { part->Release(); return; }
        handle = part;
        part->AddEffects(EF_NODRAW);
    }
    if (mount.bonemerge)
    {
        if (handle->GetMoveParent() != parent) handle->FollowEntity(parent, true);
    }
    else
    {
        Vector origin; QAngle angles;
        if (mount.bone[0])
        {
            const int bone = parent->LookupBone(mount.bone);
            if (bone < 0) return; // Missing named mounts must never float at the map origin.
            // GetBonePosition's hitbox/attachment cache can omit a valid accessory
            // bone and silently return the entity origin. Read the evaluated skeleton.
            matrix3x4_t matrices[MAXSTUDIOBONES];
            if (!parent->SetupBones(matrices, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime)) return;
            MatrixAngles(matrices[bone], angles, origin);
        }
        else { origin = parent->GetAbsOrigin(); angles = parent->GetAbsAngles(); }
        Vector forward, right, up;
        AngleVectors(angles, &forward, &right, &up);
        origin += forward * mount.position.x + right * mount.position.y + up * mount.position.z;
        RotateSWEPModuleMount(angles, up, mount.angles.y);
        AngleVectors(angles, &forward, &right, &up);
        RotateSWEPModuleMount(angles, right, mount.angles.x);
        AngleVectors(angles, &forward, &right, &up);
        RotateSWEPModuleMount(angles, forward, mount.angles.z);
        handle->SetAbsOrigin(origin);
        handle->SetAbsAngles(angles);
    }
    // Bonemerged geometry is already in the parent's skeleton coordinates.
    // TFA also ignores element.size for this case; scaling it displaces copied bones.
    const float renderScale = mount.bonemerge ? 1.0f : scale;
    if (handle->GetModelScale() != renderScale) handle->SetModelScale(renderScale);
    handle->InvalidateBoneCache();
    handle->RemoveEffects(EF_NODRAW);
    handle->DrawModel(flags);
    handle->AddEffects(EF_NODRAW);
}

void CHL2MPScriptedWeapon::DrawSWEPModules(C_BaseAnimating *parent, int flags, bool world)
{
    ApplySWEPModules(parent, world);
    if (!(flags & STUDIO_RENDER)) return;
    for (int slot = 0; slot < SCP_WEAPON_MODULE_SLOTS; ++slot)
    {
        SCPWeaponModuleOption option;
        GetSWEPModuleOption(slot, GetSWEPModuleSelection(slot), option);
        // Never sample a scope's target while that same target is being rendered.
        extern bool SCPIsRenderingWeaponScope();
        if (world && option.scopeFOV > 0 && SCPIsRenderingWeaponScope()) continue;
        CHandle<C_BaseAnimating> &handle = world ? m_hSWEPWorldParts[slot] : m_hSWEPViewParts[slot];
        DrawSWEPModuleModel(handle, parent, world ? option.worldModel : option.viewModel,
            world ? option.worldMount : option.viewMount, world ? 1.0f : option.scale, flags, world);
        if (!world)
        {
            C_BaseAnimating *lensParent = option.lensMount.relativeToModule ? handle.Get() : parent;
            if (lensParent) DrawSWEPModuleModel(m_hSWEPLensParts[slot], lensParent, option.lensModel,
                option.lensMount, option.lensScale, flags, false);
            else if (m_hSWEPLensParts[slot]) m_hSWEPLensParts[slot]->Release();
        }
    }
}

#endif


bool CHL2MPScriptedWeapon::IsNativeSWEP() const
{
    if (!L || m_nTableReference < 0) return false;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "swep_config");
    return lua_istable(L, -1);
}

Activity CHL2MPScriptedWeapon::ActivityOverride(Activity activity, bool *required)
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "swep_config");
    const bool authoredSWEP = lua_istable(L, -1);
    lua_pop(L, 1);
    const char *hold = GetAnimPrefix();
    if (authoredSWEP && (!V_stricmp(hold, "pistol") || !V_stricmp(hold, "python")))
    {
        Activity requested = activity;
        switch (activity)
        {
        case ACT_HL2MP_IDLE: case ACT_MP_STAND_IDLE: requested = ACT_IDLE_ANGRY; break;
        case ACT_HL2MP_RUN: case ACT_MP_RUN: case ACT_MP_SPRINT: requested = ACT_RUN_AIM; break;
        case ACT_MP_WALK: requested = ACT_WALK_AIM; break;
        case ACT_HL2MP_IDLE_CROUCH: case ACT_MP_CROUCH_IDLE: requested = ACT_RANGE_AIM_LOW; break;
        case ACT_HL2MP_WALK_CROUCH: case ACT_MP_CROUCHWALK: requested = ACT_WALK_CROUCH_AIM; break;
        case ACT_HL2MP_JUMP: case ACT_MP_JUMP: case ACT_MP_JUMP_START:
        case ACT_MP_JUMP_FLOAT: case ACT_MP_JUMP_LAND: requested = ACT_JUMP; break;
        case ACT_HL2MP_GESTURE_RANGE_ATTACK: case ACT_MP_ATTACK_STAND_PRIMARYFIRE:
        case ACT_MP_ATTACK_CROUCH_PRIMARYFIRE: requested = ACT_GESTURE_RANGE_ATTACK1; break;
        case ACT_HL2MP_GESTURE_RELOAD: case ACT_MP_RELOAD_STAND:
        case ACT_MP_RELOAD_CROUCH: requested = ACT_GESTURE_RELOAD; break;
        }

        struct PlayerActivity { Activity source, npc, multiplayer; };
        static const PlayerActivity pistol[] =
        {
            { ACT_IDLE, ACT_IDLE_PISTOL, ACT_HL2MP_IDLE_PISTOL },
            { ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, ACT_HL2MP_IDLE_PISTOL },
            { ACT_WALK, ACT_WALK_PISTOL, ACT_HL2MP_RUN_PISTOL },
            { ACT_RUN, ACT_RUN_PISTOL, ACT_HL2MP_RUN_PISTOL },
            { ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, ACT_HL2MP_RUN_PISTOL },
            { ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, ACT_HL2MP_RUN_PISTOL },
            { ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, ACT_HL2MP_IDLE_CROUCH_PISTOL },
            { ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, ACT_HL2MP_IDLE_CROUCH_PISTOL },
            { ACT_WALK_CROUCH, ACT_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL },
            { ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM, ACT_HL2MP_WALK_CROUCH_PISTOL },
            { ACT_JUMP, ACT_JUMP, ACT_HL2MP_JUMP_PISTOL },
            { ACT_SWIM, ACT_SWIM, ACT_HL2MP_RUN_PISTOL },
            { ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL },
            { ACT_RELOAD, ACT_RELOAD_PISTOL, ACT_HL2MP_GESTURE_RELOAD_PISTOL },
            { ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL },
            { ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, ACT_HL2MP_GESTURE_RELOAD_PISTOL }
        };
        CBaseCombatCharacter *owner = GetOwner();
        for (int i = 0; i < ARRAYSIZE(pistol); ++i)
        {
            if (pistol[i].source != requested) continue;
            const Activity candidates[] = { pistol[i].npc, pistol[i].multiplayer, requested };
            for (int n = 0; n < ARRAYSIZE(candidates); ++n)
            {
                if (!owner || owner->SelectWeightedSequence(candidates[n]) >= 0)
                {
                    if (required) *required = false;
                    return candidates[n];
                }
            }
            if (required) *required = false;
            return activity;
        }
        if (required) *required = false;
        return activity;
    }
#endif
    return BaseClass::ActivityOverride(activity, required);
}

acttable_t *CHL2MPScriptedWeapon::ActivityList( void ) {
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#ifdef LUA_SDK
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "m_acttable" );
	lua_remove( L, -2 );
	if ( lua_istable( L, -1 ) )
	{
		for( int i = 0 ; i < MIN((int)lua_objlen(L,-1),LUA_MAX_WEAPON_ACTIVITIES) ; i++ )
		{
			lua_pushinteger( L, i + 1 );
			lua_gettable( L, -2 );
			if ( lua_istable( L, -1 ) )
			{
				m_acttable[i].baseAct = ACT_INVALID;
				lua_pushinteger( L, 1 );
				lua_gettable( L, -2 );
				if ( lua_isnumber( L, -1 ) )
					m_acttable[i].baseAct = lua_tointeger( L, -1 );
				lua_pop( L, 1 );

				m_acttable[i].weaponAct = ACT_INVALID;
				lua_pushinteger( L, 2 );
				lua_gettable( L, -2 );
				if ( lua_isnumber( L, -1 ) )
					m_acttable[i].weaponAct = lua_tointeger( L, -1 );
				lua_pop( L, 1 );

				m_acttable[i].required = false;
				lua_pushinteger( L, 3 );
				lua_gettable( L, -2 );
				if ( lua_isboolean( L, -1 ) )
					m_acttable[i].required = (bool)lua_toboolean( L, -1 );
				lua_pop( L, 1 );
			}
			lua_pop( L, 1 );
		}
	}
	lua_pop( L, 1 );
#endif
	return m_acttable;
}
int CHL2MPScriptedWeapon::ActivityListCount( void ) {
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L,-1,"m_acttable");
    return lua_istable(L,-1) ? MIN((int)lua_objlen(L,-1),LUA_MAX_WEAPON_ACTIVITIES) : 0;
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHL2MPScriptedWeapon::CHL2MPScriptedWeapon( void )
{
	m_pLuaWeaponInfo = dynamic_cast< FileWeaponInfo_t* >( CreateWeaponInfo() );
    m_bSWEPAiming = false;
    m_bSWEPSafety = false;
    for (int slot = 0; slot < SCP_WEAPON_MODULE_SLOTS; ++slot) m_iSWEPModules.Set(slot, 0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHL2MPScriptedWeapon::~CHL2MPScriptedWeapon( void )
{
#ifdef CLIENT_DLL
    for (int slot = 0; slot < SCP_WEAPON_MODULE_SLOTS; ++slot)
    {
        if (m_hSWEPViewParts[slot]) m_hSWEPViewParts[slot]->Release();
        if (m_hSWEPLensParts[slot]) m_hSWEPLensParts[slot]->Release();
        if (m_hSWEPWorldParts[slot]) m_hSWEPWorldParts[slot]->Release();
    }
#endif
	delete m_pLuaWeaponInfo;
	// Andrew; This is actually done in CBaseEntity. I'm doing it here because
	// this is the class that initialized the reference.
#ifdef LUA_SDK
	lua_unref( L, m_nTableReference );
#endif
}

extern const char *pWeaponSoundCategories[ NUM_SHOOT_SOUND_TYPES ];

#ifdef CLIENT_DLL
extern ConVar hud_fastswitch;
#endif

void CHL2MPScriptedWeapon::InitScriptedWeapon( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
#ifndef CLIENT_DLL
	// Let the instance reinitialize itself for the client.
	if ( m_nTableReference != LUA_NOREF )
		return;
#endif

	char className[ MAX_WEAPON_STRING ];
#if defined ( CLIENT_DLL )
	if ( strlen( GetScriptedClassname() ) > 0 )
		Q_strncpy( className, GetScriptedClassname(), sizeof( className ) );
	else
		Q_strncpy( className, GetClassname(), sizeof( className ) );
#else
	Q_strncpy( m_iScriptedClassname.GetForModify(), GetClassname(), sizeof( className ) );
 	Q_strncpy( className, GetClassname(), sizeof( className ) );
#endif
 	Q_strlower( className );
	// Andrew; This redundancy is pretty annoying.
	// Classname
	Q_strncpy( m_pLuaWeaponInfo->szClassName, className, MAX_WEAPON_STRING );
#ifdef CLIENT_DLL
    // The client stores this pointer, so use the network member, not a stack buffer.
    if (m_iScriptedClassname.Get()[0]) SetClassname(m_iScriptedClassname.Get());
#else
    SetClassname(className);
#endif

	lua_getglobal( L, "weapon" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "get" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_remove( L, -2 );
			lua_pushstring( L, className );
			luasrc_pcall( L, 1, 1, 0 );
		}
		else
		{
			lua_pop( L, 2 );
		}
	}
	else
	{
		lua_pop( L, 1 );
	}

	m_nTableReference = luaL_ref( L, LUA_REGISTRYINDEX );
#ifndef CLIENT_DLL
	m_pLuaWeaponInfo->bParsedScript = true;
#endif

	// Printable name
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "printname" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		Q_strncpy( m_pLuaWeaponInfo->szPrintName, lua_tostring( L, -1 ), MAX_WEAPON_STRING );
	}
	else
	{
		Q_strncpy( m_pLuaWeaponInfo->szPrintName, WEAPON_PRINTNAME_MISSING, MAX_WEAPON_STRING );
	}
	lua_pop( L, 1 );
	// View model & world model
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "viewmodel" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		Q_strncpy( m_pLuaWeaponInfo->szViewModel, lua_tostring( L, -1 ), MAX_WEAPON_STRING );
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "playermodel" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		Q_strncpy( m_pLuaWeaponInfo->szWorldModel, lua_tostring( L, -1 ), MAX_WEAPON_STRING );
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "anim_prefix" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		Q_strncpy( m_pLuaWeaponInfo->szAnimationPrefix, lua_tostring( L, -1 ), MAX_WEAPON_PREFIX );
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "bucket" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iSlot = lua_tonumber( L, -1 );
	}
	else
	{
		m_pLuaWeaponInfo->iSlot = 0;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "bucket_position" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iPosition = lua_tonumber( L, -1 );
	}
	else
	{
		m_pLuaWeaponInfo->iPosition = 0;
	}
	lua_pop( L, 1 );

	// Use the console (X360) buckets if hud_fastswitch is set to 2.
#ifdef CLIENT_DLL
	if ( hud_fastswitch.GetInt() == 2 )
#else
	if ( IsX360() )
#endif
	{
		luasrc_pushweapontable(L, m_nTableReference);
		lua_getfield( L, -1, "bucket_360" );
		lua_remove( L, -2 );
		if ( lua_isnumber( L, -1 ) )
		{
			m_pLuaWeaponInfo->iSlot = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );
		luasrc_pushweapontable(L, m_nTableReference);
		lua_getfield( L, -1, "bucket_position_360" );
		lua_remove( L, -2 );
		if ( lua_isnumber( L, -1 ) )
		{
			m_pLuaWeaponInfo->iPosition = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );
	}
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "clip_size" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iMaxClip1 = lua_tonumber( L, -1 );					// Max primary clips gun can hold (assume they don't use clips by default)
	}
	else
	{
		m_pLuaWeaponInfo->iMaxClip1 = WEAPON_NOCLIP;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "clip2_size" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iMaxClip2 = lua_tonumber( L, -1 );					// Max secondary clips gun can hold (assume they don't use clips by default)
	}
	else
	{
		m_pLuaWeaponInfo->iMaxClip2 = WEAPON_NOCLIP;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "default_clip" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iDefaultClip1 = lua_tonumber( L, -1 );		// amount of primary ammo placed in the primary clip when it's picked up
	}
	else
	{
		m_pLuaWeaponInfo->iDefaultClip1 = m_pLuaWeaponInfo->iMaxClip1;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "default_clip2" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iDefaultClip2 = lua_tonumber( L, -1 );		// amount of secondary ammo placed in the secondary clip when it's picked up
	}
	else
	{
		m_pLuaWeaponInfo->iDefaultClip2 = m_pLuaWeaponInfo->iMaxClip2;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "weight" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iWeight = lua_tonumber( L, -1 );
	}
	else
	{
		m_pLuaWeaponInfo->iWeight = 0;
	}
	lua_pop( L, 1 );

	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "rumble" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->iWeight = lua_tonumber( L, -1 );
	}
	else
	{
		m_pLuaWeaponInfo->iWeight = -1;
	}
	lua_pop( L, 1 );
	
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "showusagehint" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->bShowUsageHint = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->bShowUsageHint = false;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "autoswitchto" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->bAutoSwitchTo = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->bAutoSwitchTo = true;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "autoswitchfrom" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->bAutoSwitchFrom = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->bAutoSwitchFrom = true;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "BuiltRightHanded" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->m_bBuiltRightHanded = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->m_bBuiltRightHanded = true;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "AllowFlipping" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->m_bAllowFlipping = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->m_bAllowFlipping = true;
	}
	lua_pop( L, 1 );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "MeleeWeapon" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->m_bMeleeWeapon = (int)lua_tointeger( L, -1 ) != 0 ? true : false;
	}
	else
	{
		m_pLuaWeaponInfo->m_bMeleeWeapon = false;
	}
	lua_pop( L, 1 );

	// Primary ammo used
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "primary_ammo" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		const char *pAmmo = lua_tostring( L, -1 );
		if ( strcmp("None", pAmmo) == 0 )
			Q_strncpy( m_pLuaWeaponInfo->szAmmo1, "", sizeof( m_pLuaWeaponInfo->szAmmo1 ) );
		else
			Q_strncpy( m_pLuaWeaponInfo->szAmmo1, pAmmo, sizeof( m_pLuaWeaponInfo->szAmmo1 )  );
		m_pLuaWeaponInfo->iAmmoType = GetAmmoDef()->Index( m_pLuaWeaponInfo->szAmmo1 );
	}
	lua_pop( L, 1 );
	
	// Secondary ammo used
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "secondary_ammo" );
	lua_remove( L, -2 );
	if ( lua_isstring( L, -1 ) )
	{
		const char *pAmmo = lua_tostring( L, -1 );
		if ( strcmp("None", pAmmo) == 0)
			Q_strncpy( m_pLuaWeaponInfo->szAmmo2, "", sizeof( m_pLuaWeaponInfo->szAmmo2 ) );
		else
			Q_strncpy( m_pLuaWeaponInfo->szAmmo2, pAmmo, sizeof( m_pLuaWeaponInfo->szAmmo2 )  );
		m_pLuaWeaponInfo->iAmmo2Type = GetAmmoDef()->Index( m_pLuaWeaponInfo->szAmmo2 );
	}
	lua_pop( L, 1 );

	// Now read the weapon sounds
	memset( m_pLuaWeaponInfo->aShootSounds, 0, sizeof( m_pLuaWeaponInfo->aShootSounds ) );
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "SoundData" );
	lua_remove( L, -2 );
	if ( lua_istable( L, -1 ) )
	{
		for ( int i = EMPTY; i < NUM_SHOOT_SOUND_TYPES; i++ )
		{
			lua_getfield( L, -1, pWeaponSoundCategories[i] );
			if ( lua_isstring( L, -1 ) )
			{
				const char *soundname = lua_tostring( L, -1 );
				if ( soundname && soundname[0] )
				{
					Q_strncpy( m_pLuaWeaponInfo->aShootSounds[i], soundname, MAX_WEAPON_STRING );
				}
			}
			lua_pop( L, 1 );
		}
	}
	lua_pop( L, 1 );

	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "damage" );
	lua_remove( L, -2 );
	if ( lua_isnumber( L, -1 ) )
	{
		m_pLuaWeaponInfo->m_iPlayerDamage = (int)lua_tointeger( L, -1 );
	}
	lua_pop( L, 1 );

	SetScriptedWeaponFileInfo(InsertFileWeaponInfoFromLUA(m_pLuaWeaponInfo));

	BEGIN_LUA_CALL_WEAPON_METHOD( "Initialize" );
	END_LUA_CALL_WEAPON_METHOD( 0, 0 );
#endif
}

#ifdef CLIENT_DLL
float CHL2MPScriptedWeapon::GetSWEPViewModelFOV(float fallback) const
{
    if (!L || m_nTableReference < 0) return fallback;
    LuaWeaponFrame frame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "swep_config");
    if (!lua_istable(L, -1)) return fallback;
    lua_getfield(L, -1, "ViewModelFOV");
    const float value = (float)lua_tonumber(L, -1);
    return lua_isnumber(L, -1) && IsFinite(value) ? clamp(value, 45.0f, 110.0f) : fallback;
}

// TFA's hybrid locomotion combines model sequences with a local view offset.
// Walk uses the model sequence; sprint and aiming blend their authored offsets.
static Vector SWEPViewOffset(lua_State *state, int table, const char *field)
{
    Vector result(0, 0, 0);
    lua_getfield(state, table, field);
    if (lua_istable(state, -1) || lua_isuserdata(state, -1))
    {
        const char *axes[] = { "x", "y", "z" };
        for (int i = 0; i < 3; ++i)
        {
            if (lua_istable(state, -1)) lua_rawgeti(state, -1, i + 1);
            else lua_pushnil(state);
            if (lua_isnil(state, -1))
            {
                lua_pop(state, 1);
                lua_getfield(state, -1, axes[i]);
            }
            float value = (float)lua_tonumber(state, -1);
            if (IsFinite(value)) result[i] = clamp(value, -180.0f, 180.0f);
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    return result;
}

void CHL2MPScriptedWeapon::GetAimEntOrigin(IClientEntity *attachedTo, Vector *origin, QAngle *angles)
{
    // Held world models use the animated hand, rather than the player's root.
    // MoveToAimEnt also updates culling, illumination and shadow placement.
    CBaseCombatCharacter *owner = GetOwner();
    if (L && m_nTableReference >= 0 && owner && attachedTo == owner)
    {
        LuaWeaponFrame frame(L);
        luasrc_pushweapontable(L, m_nTableReference);
        lua_getfield(L, -1, "swep_config");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "WorldModelOffset");
            if (lua_istable(L, -1))
            {
                const Vector offset = SWEPViewOffset(L, lua_gettop(L), "Position");
                const Vector rotation = SWEPViewOffset(L, lua_gettop(L), "Angles");
                const int hand = owner->LookupBone("ValveBiped.Bip01_R_Hand");
                if (hand >= 0)
                {
                    C_BaseAnimating::AutoAllowBoneAccess access(true, false);
                    matrix3x4_t bone;
                    owner->GetBoneTransform(hand, bone);
                    MatrixAngles(bone, *angles, *origin);
                    Vector forward, right, up;
                    AngleVectors(*angles, &forward, &right, &up);
                    *origin += forward * offset.x + right * offset.y + up * offset.z;
                    // Preserve the pack's order: up, then rotated right, then forward.
                    matrix3x4_t transform;
                    AngleMatrix(*angles, transform);
                    Vector axis;
                    AngleVectors(*angles, NULL, NULL, &axis);
                    matrix3x4_t turn, result;
                    MatrixBuildRotationAboutAxis(axis, rotation.x, turn);
                    ConcatTransforms(turn, transform, result);
                    MatrixAngles(result, *angles);
                    AngleVectors(*angles, NULL, &axis, NULL);
                    MatrixBuildRotationAboutAxis(axis, rotation.y, turn);
                    ConcatTransforms(turn, result, bone);
                    MatrixAngles(bone, *angles);
                    AngleVectors(*angles, &axis, NULL, NULL);
                    MatrixBuildRotationAboutAxis(axis, rotation.z, turn);
                    ConcatTransforms(turn, bone, result);
                    MatrixAngles(result, *angles);
                    return;
                }
            }
        }
    }
    BaseClass::GetAimEntOrigin(attachedTo, origin, angles);
}

// Movement is additive: drawing and reloading retain their own sequences.
static bool SWEPViewBusy(lua_State *state, int config, CBaseViewModel *viewmodel)
{
    if (!viewmodel || !viewmodel->GetModel()) return false;
    const char *current = viewmodel->GetSequenceName(viewmodel->GetSequence());
    if (!current) return false;
    const int top = lua_gettop(state);
    lua_getfield(state, config, "Animations");
    bool busy = false;
    if (lua_istable(state, -1))
    {
        const int animations = lua_gettop(state);
        lua_getfield(state, animations, "Idle");
        if (lua_istable(state, -1)) lua_getfield(state, -1, "Sequence");
        const char *idle = lua_tostring(state, -1);
        const char *actions[] = { "Draw", "DrawEmpty", "Reload", "ReloadEmpty", "ReloadStart", "ReloadFinish" };
        for (int i = 0; i < ARRAYSIZE(actions) && !busy; ++i)
        {
            const int before = lua_gettop(state);
            lua_getfield(state, animations, actions[i]);
            if (lua_istable(state, -1)) lua_getfield(state, -1, "Sequence");
            const char *sequence = lua_tostring(state, -1);
            // Melee definitions use Idle as their unused reload fallback.
            busy = sequence && (!idle || V_stricmp(sequence, idle)) && !V_stricmp(sequence, current);
            lua_settop(state, before);
        }
    }
    lua_settop(state, top);
    return busy;
}

void CHL2MPScriptedWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
    if (!L || m_nTableReference < 0) return;
    CBasePlayer *owner = ToBasePlayer(GetOwner());
    if (!owner || !owner->IsAlive()) return;

    LuaWeaponFrame luaFrame(L);
    luasrc_pushweapontable(L, m_nTableReference);
    lua_getfield(L, -1, "swep_config");
    if (!lua_istable(L, -1)) return;

    const C_BaseHLPlayer *hlPlayer = dynamic_cast<C_BaseHLPlayer *>(owner);
    const float speed = owner->GetAbsVelocity().Length2D();
    const bool grounded = owner->GetGroundEntity() && owner->GetMoveType() == MOVETYPE_WALK
        && owner->GetWaterLevel() < 2 && !owner->IsInAVehicle();
    const bool busy = SWEPViewBusy(L, lua_gettop(L), viewmodel);
    const bool sprinting = hlPlayer && hlPlayer->IsSprinting() && grounded && speed > 10 && !busy;
    const bool updateBlend = m_nSWEPViewFrame != gpGlobals->framecount;
    if (updateBlend)
    {
        // Server-authoritative ammunition changes drive visual recoil once per
        // frame, including models whose CW firing kick was entirely procedural.
        m_flSWEPRecoilBlend = MAX(0.0f, m_flSWEPRecoilBlend - gpGlobals->frametime / 0.12f);
        if (m_iSWEPPreviousClip >= 0 && m_iClip1 >= 0 && m_iClip1 < m_iSWEPPreviousClip)
            m_flSWEPRecoilBlend = 1.0f;
        m_iSWEPPreviousClip = m_iClip1;
        m_flSWEPSprintBlend = Approach(sprinting ? 1.0f : 0.0f, m_flSWEPSprintBlend,
            gpGlobals->frametime / 0.2f);
        m_flSWEPSafetyBlend = Approach(IsSafetyOn() ? 1.0f : 0.0f, m_flSWEPSafetyBlend,
            gpGlobals->frametime / 0.22f);
        const float stepTime = clamp(gpGlobals->frametime, 0.0f, 0.1f);
        const float moving = grounded && speed > 10 ? clamp(speed / 190.0f, 0.0f, 1.0f) : 0.0f;
        m_flSWEPWalkBlend = Approach(moving, m_flSWEPWalkBlend, stepTime / 0.15f);
        if (grounded)
        {
            const float stride = 140.0f + 40.0f * SimpleSpline(m_flSWEPSprintBlend);
            m_flSWEPStepPhase = fmodf(m_flSWEPStepPhase + MIN(speed, 450.0f) * stepTime * (2.0f * M_PI / stride), 2.0f * M_PI);
        }
        m_nSWEPViewFrame = gpGlobals->framecount;
    }
    const float sprint = SimpleSpline(m_flSWEPSprintBlend);
    const float safety = SimpleSpline(m_flSWEPSafetyBlend);
    float aim = 0;
    Vector position(0, 0, 0), rotation(0, 0, 0);
    Vector baseAimPosition(0, 0, 0), baseAimAngles(0, 0, 0);
    Vector safetyPosition(4.081f, 0.0f, -1.048f), safetyAngles(-5.143f, 20.809f, 0.0f);
    float walkBob = 1.0f, sprintBob = 1.0f;

    lua_getfield(L, -1, "Aim");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "Enabled"); bool enabled = lua_toboolean(L, -1) != 0; lua_pop(L, 1);
        if (enabled)
        {
            lua_getfield(L, -1, "Time");
            const float transition = clamp((float)lua_tonumber(L, -1), 0.01f, 2.0f);
            lua_pop(L, 1);
            if (updateBlend)
                m_flSWEPAimBlend = Approach(IsAiming() ? 1.0f : 0.0f, m_flSWEPAimBlend,
                    gpGlobals->frametime / transition);
            aim = SimpleSpline(m_flSWEPAimBlend) * (1.0f - safety);
            baseAimPosition = SWEPViewOffset(L, lua_gettop(L), "Position");
            baseAimAngles = SWEPViewOffset(L, lua_gettop(L), "Angles");
            position += baseAimPosition * aim;
            rotation += baseAimAngles * aim;
        }
    }
    lua_pop(L, 1);
    lua_getfield(L, 1, "Modules");
    if (lua_istable(L, -1))
    {
        const int count = MIN((int)lua_objlen(L, -1), SCP_WEAPON_MODULE_SLOTS);
        for (int slot = 0; slot < count; ++slot)
        {
            lua_rawgeti(L, -1, slot + 1);
            lua_getfield(L, -1, "Options");
            lua_rawgeti(L, -1, GetSWEPModuleSelection(slot) + 1);
            lua_getfield(L, -1, "Aim");
            if (lua_istable(L, -1))
            {
                position += (SWEPViewOffset(L, lua_gettop(L), "Position") - baseAimPosition) * aim;
                rotation += (SWEPViewOffset(L, lua_gettop(L), "Angles") - baseAimAngles) * aim;
            }
            lua_pop(L, 4);
        }
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "Motion");
    if (lua_istable(L, -1))
    {
        const int motion = lua_gettop(L);
        lua_getfield(L, motion, "WalkBobScale");
        if (lua_isnumber(L, -1)) walkBob = clamp((float)lua_tonumber(L, -1), 0.0f, 3.0f);
        lua_pop(L, 1);
        lua_getfield(L, motion, "SprintBobScale");
        if (lua_isnumber(L, -1)) sprintBob = clamp((float)lua_tonumber(L, -1), 0.0f, 3.0f);
        lua_pop(L, 1);
        lua_getfield(L, motion, "SafetyPosition");
        if (lua_istable(L, -1)) safetyPosition = SWEPViewOffset(L, motion, "SafetyPosition");
        lua_pop(L, 1);
        lua_getfield(L, motion, "SafetyAngles");
        if (lua_istable(L, -1)) safetyAngles = SWEPViewOffset(L, motion, "SafetyAngles");
        lua_pop(L, 1);
        position += SWEPViewOffset(L, motion, "BasePosition") * (1.0f - aim) * (1.0f - safety);
        rotation += SWEPViewOffset(L, motion, "BaseAngles") * (1.0f - aim) * (1.0f - safety);
        position += SWEPViewOffset(L, motion, "SprintPosition") * sprint * (1.0f - aim) * (1.0f - safety);
        rotation += SWEPViewOffset(L, lua_gettop(L), "SprintAngles") * sprint * (1.0f - aim) * (1.0f - safety);
        const float kick = SimpleSpline(m_flSWEPRecoilBlend) * (1.0f - safety) * (1.0f - 0.65f * aim);
        position += SWEPViewOffset(L, lua_gettop(L), "RecoilPosition") * kick;
        rotation += SWEPViewOffset(L, lua_gettop(L), "RecoilAngles") * kick;
    }
    // Two vertical impulses per stride, with slower lateral sway. A stopped
    // player fades to rest without snapping the phase or replacing animations.
    const float gait = SimpleSpline(m_flSWEPWalkBlend) * (1.0f - 0.9f * aim)
        * (1.0f - 0.65f * safety) * (busy ? 0.2f : 1.0f) * Lerp(sprint, walkBob, sprintBob);
    const float side = sinf(m_flSWEPStepPhase);
    const float step = cosf(2.0f * m_flSWEPStepPhase);
    position += Vector(side * (0.28f + 0.18f * sprint), step * 0.08f,
        step * (0.20f + 0.22f * sprint)) * gait;
    rotation += Vector(step * (0.35f + 0.4f * sprint), side * 0.35f,
        side * (0.55f + 0.35f * sprint)) * gait;
    Vector forward, right, up;
    AngleVectors(angles, &forward, &right, &up);
    origin += right * position.x + forward * position.y + up * position.z;
    // Rotate in the view's local axes, matching authored TFA sight offsets.
    matrix3x4_t basis, offset, combined;
    AngleMatrix(angles, basis);
    AngleMatrix(QAngle(rotation.x, rotation.y, rotation.z), offset);
    ConcatTransforms(basis, offset, combined);
    MatrixAngles(combined, angles);
    if (safety > 0.0f)
    {
        // The SCP5K pack uses SafetyPos (4.081,0,-1.048) and SafetyAng
        // (-5.143,20.809,0). TFA rotates around right/up/forward in that
        // order, then translates along the rotated axes, not the eye axes.
        const float turns[] = { safetyAngles.x * safety, safetyAngles.y * safety, safetyAngles.z * safety };
        matrix3x4_t pose;
        AngleMatrix(angles, pose);
        for (int i = 0; i < ARRAYSIZE(turns); ++i)
        {
            AngleVectors(angles, &forward, &right, &up);
            const Vector axis = i == 0 ? right : (i == 1 ? up : forward);
            matrix3x4_t turn, result;
            MatrixBuildRotationAboutAxis(axis, turns[i], turn);
            ConcatTransforms(turn, pose, result);
            MatrixCopy(result, pose);
            MatrixAngles(pose, angles);
        }
        AngleVectors(angles, &forward, &right, &up);
        origin += (right * safetyPosition.x + forward * safetyPosition.y + up * safetyPosition.z) * safety;
    }
}

void CHL2MPScriptedWeapon::OnDataChanged( DataUpdateType_t updateType )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		if ( m_iScriptedClassname.Get() && !m_pLuaWeaponInfo->bParsedScript )
		{
			m_pLuaWeaponInfo->bParsedScript = true;
			SetClassname( m_iScriptedClassname.Get() );
			InitScriptedWeapon();

#ifdef LUA_SDK
			BEGIN_LUA_CALL_WEAPON_METHOD( "Precache" );
			END_LUA_CALL_WEAPON_METHOD( 0, 0 );
#endif
		}
	}
}

const char *CHL2MPScriptedWeapon::GetScriptedClassname( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	if ( m_iScriptedClassname.Get() )
		return m_iScriptedClassname.Get();
	return BaseClass::GetClassname();
}
#endif

void CHL2MPScriptedWeapon::Precache( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	BaseClass::Precache();

	InitScriptedWeapon();

	// Get the ammo indexes for the ammo's specified in the data file
	if ( GetWpnData().szAmmo1[0] )
	{
		m_iPrimaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo1 );
		if (m_iPrimaryAmmoType == -1)
		{
			Msg("ERROR: Weapon (%s) using undefined primary ammo type (%s)\n",GetClassname(), GetWpnData().szAmmo1);
		}
	}
	if ( GetWpnData().szAmmo2[0] )
	{
		m_iSecondaryAmmoType = GetAmmoDef()->Index( GetWpnData().szAmmo2 );
		if (m_iSecondaryAmmoType == -1)
		{
			Msg("ERROR: Weapon (%s) using undefined secondary ammo type (%s)\n",GetClassname(),GetWpnData().szAmmo2);
		}

	}

	// Precache models (preload to avoid hitch)
	m_iViewModelIndex = 0;
	m_iWorldModelIndex = 0;
	if ( GetViewModel() && GetViewModel()[0] )
	{
		m_iViewModelIndex = CBaseEntity::PrecacheModel( GetViewModel() );
	}
	if ( GetWorldModel() && GetWorldModel()[0] )
	{
		m_iWorldModelIndex = CBaseEntity::PrecacheModel( GetWorldModel() );
	}

	// Precache sounds, too
	for ( int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i )
	{
		const char *shootsound = GetShootSound( i );
		if ( shootsound && shootsound[0] )
		{
			CBaseEntity::PrecacheScriptSound( shootsound );
		}
	}

#if defined ( LUA_SDK ) && !defined( CLIENT_DLL )
	BEGIN_LUA_CALL_WEAPON_METHOD( "Precache" );
	END_LUA_CALL_WEAPON_METHOD( 0, 0 );
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Get my data in the file weapon info array
//-----------------------------------------------------------------------------
const FileWeaponInfo_t &CHL2MPScriptedWeapon::GetWpnData( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	return *m_pLuaWeaponInfo;
}

const char *CHL2MPScriptedWeapon::GetViewModel( int ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "viewmodel" );
	lua_remove( L, -2 );

	RETURN_LUA_STRING();
#endif

	return BaseClass::GetViewModel();
}

const char *CHL2MPScriptedWeapon::GetWorldModel( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "playermodel" );
	lua_remove( L, -2 );

	RETURN_LUA_STRING();
#endif

	return BaseClass::GetWorldModel();
}

const char *CHL2MPScriptedWeapon::GetAnimPrefix( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "anim_prefix" );
	lua_remove( L, -2 );

	RETURN_LUA_STRING();
#endif

	return BaseClass::GetAnimPrefix();
}

const char *CHL2MPScriptedWeapon::GetPrintName( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "printname" );
	lua_remove( L, -2 );

	RETURN_LUA_STRING();
#endif

	return BaseClass::GetPrintName();
}

int CHL2MPScriptedWeapon::GetMaxClip1( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "clip_size" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetMaxClip1();
}

int CHL2MPScriptedWeapon::GetMaxClip2( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "clip2_size" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetMaxClip2();
}

int CHL2MPScriptedWeapon::GetDefaultClip1( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "default_clip" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetDefaultClip1();
}

int CHL2MPScriptedWeapon::GetDefaultClip2( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "default_clip2" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetDefaultClip2();
}


bool CHL2MPScriptedWeapon::IsMeleeWeapon() const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "MeleeWeapon" );
	lua_remove( L, -2 );

	if ( lua_gettop( L ) == 1 )
	{
		if ( lua_isnumber( L, -1 ) )
		{
			int res = ( (int)lua_tointeger( L, -1 ) != 0 ) ? true : false;
			lua_pop(L, 1);
			return res;
		}
		else
			lua_pop(L, 1);
	}
#endif

	return BaseClass::IsMeleeWeapon();
}

int CHL2MPScriptedWeapon::GetWeight( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "weight" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetWeight();
}

bool CHL2MPScriptedWeapon::AllowsAutoSwitchTo( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "autoswitchto" );
	lua_remove( L, -2 );

	if ( lua_gettop( L ) == 1 )
	{
		if ( lua_isnumber( L, -1 ) )
		{
			int res = ( (int)lua_tointeger( L, -1 ) != 0 ) ? true : false;
			lua_pop(L, 1);
			return res;
		}
		else
			lua_pop(L, 1);
	}
#endif

	return BaseClass::AllowsAutoSwitchTo();
}

bool CHL2MPScriptedWeapon::AllowsAutoSwitchFrom( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "autoswitchfrom" );
	lua_remove( L, -2 );

	if ( lua_gettop( L ) == 1 )
	{
		if ( lua_isnumber( L, -1 ) )
		{
			int res = ( (int)lua_tointeger( L, -1 ) != 0 ) ? true : false;
			lua_pop(L, 1);
			return res;
		}
		else
			lua_pop(L, 1);
	}
#endif

	return BaseClass::AllowsAutoSwitchFrom();
}

int CHL2MPScriptedWeapon::GetWeaponFlags( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "item_flags" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetWeaponFlags();
}

int CHL2MPScriptedWeapon::GetSlot( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "bucket" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetSlot();
}

int CHL2MPScriptedWeapon::GetPosition( void ) const
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	luasrc_pushweapontable(L, m_nTableReference);
	lua_getfield( L, -1, "bucket_position" );
	lua_remove( L, -2 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::GetPosition();
}

const Vector &CHL2MPScriptedWeapon::GetBulletSpread( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	static Vector cone = VECTOR_CONE_3DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHL2MPScriptedWeapon::PrimaryAttack( void )
{
    if (IsSafetyOn()) return;
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "PrimaryAttack" );
	END_LUA_CALL_WEAPON_METHOD( 0, 0 );
#endif
}

void CHL2MPScriptedWeapon::SecondaryAttack( void )
{
    if (IsSafetyOn()) return;
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "SecondaryAttack" );
	END_LUA_CALL_WEAPON_METHOD( 0, 0 );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CHL2MPScriptedWeapon::FireBullets( const FireBulletsInfo_t &info )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

	if(CBasePlayer *pPlayer = ToBasePlayer ( GetOwner() ) )
	{
		pPlayer->FireBullets(info);
	}
}

bool CHL2MPScriptedWeapon::Reload( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "Reload" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	RETURN_LUA_BOOLEAN();
#endif
	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2MPScriptedWeapon::Deploy( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "Deploy" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	RETURN_LUA_BOOLEAN();
#endif

	return BaseClass::Deploy();
}

Activity CHL2MPScriptedWeapon::GetDrawActivity( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "GetDrawActivity" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	// Kind of lame, but we're required to explicitly cast
	RETURN_LUA_ACTIVITY();
#endif

	return BaseClass::GetDrawActivity();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2MPScriptedWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "Holster" );
		lua_pushweapon( L, pSwitchingTo );
	END_LUA_CALL_WEAPON_METHOD( 1, 1 );

	RETURN_LUA_BOOLEAN();
#endif

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPScriptedWeapon::ItemPostFrame( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "ItemPostFrame" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	RETURN_LUA_NONE();
#endif

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Called each frame by the player PostThink, if the player's not ready to attack yet
//-----------------------------------------------------------------------------
void CHL2MPScriptedWeapon::ItemBusyFrame( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "ItemBusyFrame" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	RETURN_LUA_NONE();
#endif

	BaseClass::ItemBusyFrame();
}

#ifndef CLIENT_DLL
int CHL2MPScriptedWeapon::CapabilitiesGet( void )
{
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "CapabilitiesGet" );
	END_LUA_CALL_WEAPON_METHOD( 0, 1 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::CapabilitiesGet();
}
#else
int CHL2MPScriptedWeapon::DrawModel( int flags )
{
    DrawSWEPModules(this, flags, true);
#ifdef LUA_SDK
    LuaWeaponFrame luaFrame(L);
#endif

#if defined ( LUA_SDK )
	BEGIN_LUA_CALL_WEAPON_METHOD( "DrawModel" );
		lua_pushinteger( L, flags );
	END_LUA_CALL_WEAPON_METHOD( 1, 1 );

	RETURN_LUA_INTEGER();
#endif

	return BaseClass::DrawModel( flags );
}
#endif


