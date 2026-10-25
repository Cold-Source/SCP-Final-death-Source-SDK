#include "cbase.h"
#include "hudelement.h"
#include "hud.h"
#include "clientmode.h"
#include "hl2/c_basehlplayer.h"
#include "ienginevgui.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

// Presentation only: pickup time, aim, distance and ownership are server-authoritative.
class CHudWeaponPickup : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudWeaponPickup, vgui::Panel);
public:
    CHudWeaponPickup(const char *name) : CHudElement(name), BaseClass(NULL, "HudWeaponPickup")
    {
        SetParent(g_pClientMode->GetViewport());
        SetPaintBackgroundEnabled(false);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
        SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_ALL);
    }

    bool ShouldDraw() OVERRIDE
    {
        C_BaseHLPlayer *player = dynamic_cast<C_BaseHLPlayer *>(C_BasePlayer::GetLocalPlayer());
        return player && player->IsAlive() && !player->IsObserver() &&
            player->m_flWeaponPickupProgress >= 0.0f &&
            !engine->IsLevelMainMenuBackground() && !engine->Con_IsVisible() &&
            !enginevgui->IsGameUIVisible() && CHudElement::ShouldDraw();
    }

    void OnThink() OVERRIDE
    {
        int w, h;
        vgui::surface()->GetScreenSize(w, h);
        SetBounds(0, 0, w, h);
    }

    void Paint() OVERRIDE
    {
        C_BaseHLPlayer *player = dynamic_cast<C_BaseHLPlayer *>(C_BasePlayer::GetLocalPlayer());
        if (!player) return;
        const float progress = clamp(player->m_flWeaponPickupProgress, 0.0f, 1.0f);
        const float scale = MAX(0.8f, GetTall() / 720.0f);
        const int x = GetWide() / 2;
        const int y = GetTall() / 2;
        const int radius = MAX(18, (int)(22.0f * scale));
        const int thickness = MAX(2, (int)(3.0f * scale));
        const int segments = 128;
        for (int i = 0; i < segments; ++i)
        {
            const float a = -M_PI_F / 2.0f + i * (2.0f * M_PI_F / segments);
            const float b = -M_PI_F / 2.0f + (i + 1) * (2.0f * M_PI_F / segments);
            // Wide translucent black track; a narrow white arc fills clockwise.
            vgui::surface()->DrawSetColor(0, 0, 0, 115);
            for (int t = -1; t <= thickness; ++t)
            {
                const float r = (float)(radius + t);
                vgui::surface()->DrawLine(x + (int)(cosf(a) * r), y + (int)(sinf(a) * r),
                    x + (int)(cosf(b) * r), y + (int)(sinf(b) * r));
            }
            vgui::surface()->DrawSetColor(255, 255, 255, i < progress * segments ? 225 : 55);
            for (int t = 0; t < thickness; ++t)
            {
                const float r = (float)(radius + t);
                vgui::surface()->DrawLine(x + (int)(cosf(a) * r), y + (int)(sinf(a) * r),
                    x + (int)(cosf(b) * r), y + (int)(sinf(b) * r));
            }
        }
    }
};

DECLARE_HUDELEMENT(CHudWeaponPickup);
