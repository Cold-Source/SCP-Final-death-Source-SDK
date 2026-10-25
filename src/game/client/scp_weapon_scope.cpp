#include "cbase.h"
#include "baseclientrendertargets.h"
#include "hl2mp/weapon_hl2mpbase_scriptedweapon.h"
#include "view_shared.h"
#include "viewrender.h"
#include "view_scene.h"
#include "rendertexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "tier1/KeyValues.h"

static bool s_RenderingWeaponScope = false;
bool SCPIsRenderingWeaponScope() { return s_RenderingWeaponScope; }

// Allocate at the engine's render-target initialization boundary, never in DrawModel.
class CSCPWeaponRenderTargets : public CBaseClientRenderTargets
{
public:
    void InitClientRenderTargets(IMaterialSystem *system, IMaterialSystemHardwareConfig *hardware) OVERRIDE
    {
        CBaseClientRenderTargets::InitClientRenderTargets(system, hardware);
        m_Scope.Init(system->CreateNamedRenderTargetTextureEx2("_rt_SCPWeaponScope",
            512, 512, RT_SIZE_NO_CHANGE, system->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SEPARATE,
            TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR));
    }
    void ShutdownClientRenderTargets() OVERRIDE
    {
        m_Reticle.Shutdown();
        m_LensReticle.Shutdown();
        m_ReticleName[0] = 0;
        m_Scope.Shutdown();
        CBaseClientRenderTargets::ShutdownClientRenderTargets();
    }
    void Update(CViewRender &renderer, const CViewSetup &view, float fov, bool diskReticle)
    {
        if (!m_Scope.IsValid() || view.width <= 0 || view.height <= 0) return;
        if (fov > 0)
        {
            s_RenderingWeaponScope = true;
            renderer.DrawWeaponScopeView(view, m_Scope, fov);
            s_RenderingWeaponScope = false;
        }
        CMatRenderContextPtr context(materials);
        context->PushRenderTargetAndViewport(m_Scope, 0, 0, 512, 512);
        if (fov <= 0)
        {
            context->ClearColor4ub(0, 0, 0, 255);
            context->ClearBuffers(true, false);
        }
        if (diskReticle)
        {
            if (!m_LensReticle.IsValid()) m_LensReticle.Init("models/weapons/tfa_ins2/optics/mk4_crosshair", TEXTURE_GROUP_MODEL);
            if (!m_LensReticle->IsErrorMaterial())
                context->DrawScreenSpaceRectangle(m_LensReticle, 0, 0, 512, 512, 0, 0,
                    m_LensReticle->GetMappingWidth() - 1, m_LensReticle->GetMappingHeight() - 1,
                    m_LensReticle->GetMappingWidth(), m_LensReticle->GetMappingHeight());
        }
        context->PopRenderTargetAndViewport();
    }
    void DrawReticle(const CViewSetup &view, const SCPWeaponModuleOption &option)
    {
        if (V_strcmp(m_ReticleName, option.reticle))
        {
            V_strncpy(m_ReticleName, option.reticle, sizeof(m_ReticleName));
            char material[128];
            V_StripExtension(option.reticle, material, sizeof(material));
            m_Reticle.Init(material, TEXTURE_GROUP_MODEL);
        }
        if (!m_Reticle.IsValid() || m_Reticle->IsErrorMaterial()) return;
        const int size = MAX(4, (int)(option.reticleSize * view.height / 720.0f));
        CMatRenderContextPtr context(materials);
        context->DrawScreenSpaceRectangle(m_Reticle, view.x + (view.width - size) / 2,
            view.y + (view.height - size) / 2, size, size, 0, 0,
            m_Reticle->GetMappingWidth() - 1, m_Reticle->GetMappingHeight() - 1,
            m_Reticle->GetMappingWidth(), m_Reticle->GetMappingHeight());
    }
private:
    char m_ReticleName[128] = {};
    CMaterialReference m_Reticle, m_LensReticle;
    CTextureReference m_Scope;
};

static CSCPWeaponRenderTargets g_SCPWeaponRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSCPWeaponRenderTargets, IClientRenderTargets,
    CLIENTRENDERTARGETS_INTERFACE_VERSION, g_SCPWeaponRenderTargets);

void SCPUpdateWeaponScope(CViewRender &renderer, const CViewSetup &view)
{
    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if (!player || !player->IsAlive() || player->ShouldDrawLocalPlayer()) return;
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(player->GetActiveWeapon());
    if (!weapon || !weapon->IsNativeSWEP()) return;
    bool diskReticle = false;
    float fov = 0.0f;
    for (int i = 0; i < weapon->GetSWEPModuleCount(); ++i)
    {
        SCPWeaponModuleOption option;
        if (weapon->GetSWEPModuleOption(i, weapon->GetSWEPModuleSelection(i), option) && option.scopeFOV > 0)
        { fov = option.scopeFOV; diskReticle = option.lensModel[0] != 0; }
    }
    g_SCPWeaponRenderTargets.Update(renderer, view, weapon->IsAiming() ? fov : 0.0f, diskReticle);
}

// Holographic reticles are projected at the authoritative shooting centre only
// when the authored ADS transition is complete. No dependency on GMod drawing hooks.
void SCPDrawWeaponReticle(const CViewSetup &view)
{
    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if (!player || !player->IsAlive() || player->ShouldDrawLocalPlayer()) return;
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(player->GetActiveWeapon());
    if (!weapon || !weapon->IsNativeSWEP() || !weapon->IsAiming() || weapon->GetSWEPAimFraction() < 0.98f) return;
    for (int i = 0; i < weapon->GetSWEPModuleCount(); ++i)
    {
        SCPWeaponModuleOption option;
        if (weapon->GetSWEPModuleOption(i, weapon->GetSWEPModuleSelection(i), option) && option.reticle[0])
        { g_SCPWeaponRenderTargets.DrawReticle(view, option); break; }
    }
}
