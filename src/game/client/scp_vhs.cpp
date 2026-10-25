#include "cbase.h"
#include "ScreenSpaceEffects.h"
#include "clientmode.h"
#include "hudelement.h"
#include "hud.h"
#include "ienginevgui.h"
#include "view_scene.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "materialsystem/MaterialSystemUtil.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include <time.h>

static ConVar scp_vhs("scp_vhs", "0", FCVAR_ARCHIVE, "Enable the native VHS camera effect.");
static ConVar scp_vhs_strength("scp_vhs_strength", "1", FCVAR_ARCHIVE,
    "VHS signal degradation strength.", true, 0.0f, true, 1.0f);
static ConVar scp_vhs_timestamp("scp_vhs_timestamp", "1", FCVAR_ARCHIVE,
    "Show the VHS camera date and time.");

static bool VHSWorldVisible()
{
    static ConVarRef cubemaps("building_cubemaps");
    return scp_vhs.GetBool() && scp_vhs_strength.GetFloat() > 0.0f &&
        engine->IsInGame() && !engine->IsLevelMainMenuBackground() &&
        !(cubemaps.IsValid() && cubemaps.GetBool());
}

// The existing screen-effect stage runs after the scene AND viewmodels, before
// VGUI. This keeps menus and gameplay HUD readable and filters the weapon too.
class CSCPVHSEffect : public IScreenSpaceEffect
{
public:
    void Init() OVERRIDE
    {
        m_Material.Init("effects/scp_vhs", TEXTURE_GROUP_OTHER);
        m_Ready = m_Material && !m_Material->IsErrorMaterial();
        const char *names[] = { "$c0_x", "$c0_y", "$c0_z", "$c0_w",
            "$c1_x", "$c1_y", "$c1_z", "$c1_w", "$c2_x" };
        for (int i = 0; i < ARRAYSIZE(names) && m_Ready; ++i)
        {
            bool found = false;
            m_Params[i] = m_Material->FindVar(names[i], &found, false);
            m_Ready = found;
        }
        if (!m_Ready) Warning("SCP VHS: effects/scp_vhs material is unavailable; effect skipped.\n");
    }

    void Shutdown() OVERRIDE { m_Ready = false; m_Material.Shutdown(); }
    void Enable(bool enabled) OVERRIDE { m_Enabled = enabled; }
    bool IsEnabled() OVERRIDE { return m_Enabled && scp_vhs.GetBool(); }
    void SetParameters(KeyValues *) OVERRIDE {}

    void Render(int x, int y, int w, int h) OVERRIDE
    {
        if (!IsEnabled() || !m_Ready || !VHSWorldVisible() || w < 64 || h < 64) return;
        Rect_t source;
        UpdateScreenEffectTexture(0, x, y, w, h, false, &source);
        ITexture *texture = GetFullFrameFrameBufferTexture(0);
        if (!texture || texture->IsError()) return;
        const float tw = (float)texture->GetActualWidth();
        const float th = (float)texture->GetActualHeight();
        if (tw <= 0.0f || th <= 0.0f || source.width <= 0 || source.height <= 0) return;
        const float frameWidth = MIN((float)w, h * (4.0f / 3.0f));
        const float frameFraction = frameWidth / w;
        const float values[] = { fmodf(gpGlobals->realtime, 4096.0f), 720.0f / frameFraction, 576.0f,
            scp_vhs_strength.GetFloat(), source.x / tw, source.y / th,
            source.width / tw, source.height / th, frameFraction };
        for (int i = 0; i < ARRAYSIZE(values); ++i) m_Params[i]->SetFloatValue(values[i]);
        CMatRenderContextPtr context(materials);
        context->DrawScreenSpaceRectangle(m_Material, x, y, w, h,
            source.x, source.y, source.x + source.width - 1, source.y + source.height - 1,
            texture->GetActualWidth(), texture->GetActualHeight());
    }

private:
    CMaterialReference m_Material;
    IMaterialVar *m_Params[9] = {};
    bool m_Ready = false;
    bool m_Enabled = true;
};

ADD_SCREENSPACE_EFFECT(CSCPVHSEffect, scp_vhs_camera);

class CHudVHSTimestamp : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudVHSTimestamp, vgui::Panel);
public:
    CHudVHSTimestamp(const char *name) : CHudElement(name), BaseClass(NULL, "HudVHSTimestamp")
    {
        SetParent(g_pClientMode->GetViewport());
        SetPaintBackgroundEnabled(false);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
        SetHiddenBits(HIDEHUD_ALL);
    }

    void VidInit() OVERRIDE { m_FontTall = 0; }
    bool ShouldDraw() OVERRIDE
    {
        return VHSWorldVisible() && scp_vhs_timestamp.GetBool() &&
            !engine->Con_IsVisible() && !enginevgui->IsGameUIVisible() && CHudElement::ShouldDraw();
    }

    void OnThink() OVERRIDE
    {
        int w, h;
        vgui::surface()->GetScreenSize(w, h);
        SetBounds(0, 0, w, h);
        const int tall = clamp((int)(36.0f * h / 720.0f), 18, 120);
        if (m_FontTall != tall)
        {
            vgui::surface()->AddCustomFontFile("Real Vhs Font", "resource/fonts/realvhsfont_regular.ttf");
            if (!m_Font) m_Font = vgui::surface()->CreateFont();
            if (!vgui::surface()->SetFontGlyphSet(m_Font, "Real Vhs Font", tall, 500, 0, 0,
                vgui::ISurface::FONTFLAG_ANTIALIAS | vgui::ISurface::FONTFLAG_CUSTOM, 0x20, 0x7E))
                vgui::surface()->SetFontGlyphSet(m_Font, "Consolas", tall, 500, 0, 0,
                    vgui::ISurface::FONTFLAG_ANTIALIAS, 0x20, 0x7E);
            m_FontTall = tall;
        }
    }

    void Paint() OVERRIDE
    {
        if (!m_Font) return;
        const time_t stamp = time(NULL);
        if (stamp != m_LastStamp)
        {
            struct tm date;
#ifdef _WIN32
            if (localtime_s(&date, &stamp) != 0) return;
#else
            if (!localtime_r(&stamp, &date)) return;
#endif
            V_snwprintf(m_Time, ARRAYSIZE(m_Time), L"%d:%02d:%02d", date.tm_hour, date.tm_min, date.tm_sec);
            V_snwprintf(m_Date, ARRAYSIZE(m_Date), L"%d. %d.%04d", date.tm_mday, date.tm_mon + 1, date.tm_year + 1900);
            m_LastStamp = stamp;
        }
        const int frameWidth = MIN(GetWide(), (GetTall() * 4) / 3);
        const int frameLeft = (GetWide() - frameWidth) / 2;
        const int x = frameLeft + (int)(frameWidth * 0.08f);
        const int y = GetTall() - (int)(m_FontTall * 3.5f);
        DrawStamp(m_Time, x, y);
        DrawStamp(m_Date, x, y + m_FontTall);
    }

private:
    void DrawStamp(const wchar_t *text, int x, int y)
    {
        vgui::surface()->DrawSetTextFont(m_Font);
        vgui::surface()->DrawSetTextColor(0, 0, 0, 170);
        vgui::surface()->DrawSetTextPos(x + 2, y + 2);
        vgui::surface()->DrawPrintText(text, V_wcslen(text));
        vgui::surface()->DrawSetTextColor(255, 255, 255, 65);
        vgui::surface()->DrawSetTextPos(x + MAX(1, m_FontTall / 18), y);
        vgui::surface()->DrawPrintText(text, V_wcslen(text));
        vgui::surface()->DrawSetTextColor(245, 245, 245, 230);
        vgui::surface()->DrawSetTextPos(x, y);
        vgui::surface()->DrawPrintText(text, V_wcslen(text));
    }
    vgui::HFont m_Font = 0;
    int m_FontTall = 0;
    time_t m_LastStamp = 0;
    wchar_t m_Time[32] = {};
    wchar_t m_Date[32] = {};
};

DECLARE_HUDELEMENT(CHudVHSTimestamp);
