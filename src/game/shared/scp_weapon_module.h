#pragma once

enum { SCP_WEAPON_MODULE_SLOTS = 8, SCP_WEAPON_MODULE_OPTIONS = 16 };
struct SCPWeaponModuleMount
{
    char bone[64] = {};
    Vector position = Vector(0, 0, 0);
    Vector angles = Vector(0, 0, 0);
    bool bonemerge = true;
    bool relativeToModule = false;
};
struct SCPWeaponModuleOption
{
    char name[96] = {};
    char viewModel[128] = {};
    char worldModel[128] = {};
    char reticle[128] = {};
    float reticleSize = 64.0f;
    float scopeFOV = 0.0f;
    float scale = 1.0f;
    SCPWeaponModuleMount viewMount, worldMount, lensMount;
    char lensModel[128] = {};
    float lensScale = 1.0f;
    int bodygroup = -1;
    int bodyValue = 0;
    int worldBodygroup = -1;
    int worldBodyValue = 0;
};
