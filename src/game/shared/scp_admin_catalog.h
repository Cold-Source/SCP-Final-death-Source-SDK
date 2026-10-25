#ifndef SCP_ADMIN_CATALOG_H
#define SCP_ADMIN_CATALOG_H
#ifdef _WIN32
#pragma once
#endif

// Shared built-in catalog and identifier check. Custom classes also require
// a successful server-side lookup in the active Lua weapon registry.
inline bool SCPAdminWeaponClassname(const char *name)
{
    if (!name || !*name) return false;
    int length = 0;
    for (; *name; ++name)
    {
        if (++length >= 80 || !((*name >= 'a' && *name <= 'z') ||
            (*name >= '0' && *name <= '9') || *name == '_')) return false;
    }
    return true;
}

struct SCPAdminSpawn
{
    const char *classname;
    const char *model;
    const char *name;
    const char *equipment;
};

static const SCPAdminSpawn SCPAdminNPCs[] =
{
    { "npc_headcrab", "models/headcrabclassic.mdl", "#SCPAdmin_Headcrab", "" },
    { "npc_antlion", "models/antlion.mdl", "#SCPAdmin_Antlion", "" },
    { "npc_manhack", "models/manhack.mdl", "#SCPAdmin_Manhack", "" },
};

static const SCPAdminSpawn SCPAdminWeapons[] =
{
    { "weapon_crowbar", "models/weapons/w_crowbar.mdl", "#SCPAdmin_Crowbar", "" },
    { "weapon_pistol", "models/weapons/w_pistol.mdl", "#SCPAdmin_Pistol", "" },
    { "weapon_357", "models/weapons/w_357.mdl", "#SCPAdmin_Revolver", "" },
    { "weapon_smg1", "models/weapons/w_smg1.mdl", "SMG", "" },
    { "weapon_ar2", "models/weapons/w_irifle.mdl", "AR2", "" },
    { "weapon_shotgun", "models/weapons/w_shotgun.mdl", "#SCPAdmin_Shotgun", "" },
    { "weapon_crossbow", "models/weapons/w_crossbow.mdl", "#SCPAdmin_Crossbow", "" },
    { "weapon_rpg", "models/weapons/w_rocket_launcher.mdl", "RPG", "" },
    { "weapon_frag", "models/weapons/w_grenade.mdl", "#SCPAdmin_Grenade", "" },
    { "weapon_physcannon", "models/weapons/w_physics.mdl", "#SCPAdmin_GravityGun", "" },
};

#endif
