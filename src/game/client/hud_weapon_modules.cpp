#include "cbase.h"
#include "clientmode.h"
#include "hl2mp/weapon_hl2mpbase_scriptedweapon.h"
#include <vgui/ISurface.h>
#include <vgui_controls/EditablePanel.h>

// This popup owns its presentation. It must not depend on GameUI's Frame,
// Marlett symbols, borders or combo-box scheme being installed in ClientScheme.
class CWeaponModuleMenu : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CWeaponModuleMenu, vgui::EditablePanel);
public:
    CWeaponModuleMenu(CHL2MPScriptedWeapon *weapon) :
        BaseClass(g_pClientMode->GetViewport(), "WeaponModules"), m_Weapon(weapon)
    {
        int w, h;
        vgui::surface()->GetScreenSize(w, h);
        const int width = MIN(720, w - 32);
        m_Count = weapon->GetSWEPModuleCount();
        m_RowHeight = MIN(46, MAX(30, (h - 32 - 136) / MAX(1, m_Count)));
        const int height = MIN(h - 32, 136 + MAX(1, m_Count) * m_RowHeight);
        SetBounds((w - width) / 2, (h - height) / 2, width, height);
        SetPaintBackgroundEnabled(false);
        SetPaintBorderEnabled(false);
        for (int i = 0; i < m_Count; ++i) m_Selected[i] = weapon->GetSWEPModuleSelection(i);
        MakePopup();
        SetKeyBoardInputEnabled(true);
        SetMouseInputEnabled(true);
        SetVisible(true);
        MoveToFront();
        RequestFocus();
    }

    void Close() { SetVisible(false); MarkForDeletion(); }

    void ApplySchemeSettings(vgui::IScheme *scheme) OVERRIDE
    {
        BaseClass::ApplySchemeSettings(scheme);
        // Use pixel-sized, complete Cyrillic fonts rather than unresolved scheme handles.
        static vgui::HFont text = 0, title = 0;
        if (!text)
        {
            text = vgui::surface()->CreateFont();
            vgui::surface()->SetFontGlyphSet(text, "Trebuchet MS", 18, 700, 0, 0,
                vgui::ISurface::FONTFLAG_ANTIALIAS, 0x20, 0x4ff);
            title = vgui::surface()->CreateFont();
            vgui::surface()->SetFontGlyphSet(title, "Trebuchet MS", 24, 700, 0, 0,
                vgui::ISurface::FONTFLAG_ANTIALIAS, 0x20, 0x4ff);
        }
        m_Font = text;
        m_TitleFont = title;
        SetPaintBackgroundEnabled(false);
        SetPaintBorderEnabled(false);
    }

    void Paint() OVERRIDE
    {
        if (!m_Weapon || !m_Font) return;
        vgui::ISurface *surface = vgui::surface();
        surface->DrawSetColor(8, 8, 8, 242);
        surface->DrawFilledRect(0, 0, GetWide(), GetTall());
        surface->DrawSetColor(255, 255, 255, 180);
        surface->DrawOutlinedRect(0, 0, GetWide(), GetTall());
        Text(22, 16, L"\u041c\u041e\u0414\u0423\u041b\u0418 \u041e\u0420\u0423\u0416\u0418\u042f", m_TitleFont);
        Text(GetWide() - 38, 16, L"X", m_TitleFont);
        TextUTF8(22, 50, m_Weapon->GetPrintName());
        const int split = GetWide() / 3;
        for (int slot = 0; slot < m_Count; ++slot)
        {
            const int y = Top + slot * m_RowHeight;
            const bool hover = slot == m_Hover;
            surface->DrawSetColor(255, 255, 255, hover ? 235 : 25);
            surface->DrawFilledRect(split, y, GetWide() - 20, y + m_RowHeight - 6);
            char name[96];
            m_Weapon->GetSWEPModuleName(slot, name, sizeof(name));
            TextUTF8(22, y + 10, name, false, split - 44);
            SCPWeaponModuleOption option;
            if (m_Weapon->GetSWEPModuleOption(slot, m_Selected[slot], option))
                TextUTF8(split + 32, y + 10, option.name, hover, GetWide() - split - 84);
            Text(split + 10, y + 10, L"<", m_Font, hover);
            Text(GetWide() - 40, y + 10, L">", m_Font, hover);
        }
        if (!m_Count) Text(22, Top + 10, L"\u0414\u043b\u044f \u044d\u0442\u043e\u0433\u043e \u043e\u0440\u0443\u0436\u0438\u044f \u043d\u0435\u0442 \u043c\u043e\u0434\u0443\u043b\u0435\u0439", m_Font);
        Text(22, GetTall() - 30, L"C / Esc: \u0437\u0430\u043a\u0440\u044b\u0442\u044c     < >: \u0432\u044b\u0431\u043e\u0440 \u043c\u043e\u0434\u0443\u043b\u044f", m_Font);
    }

    void OnCursorMoved(int x, int y) OVERRIDE
    {
        m_Hover = x >= GetWide() / 3 && x < GetWide() - 20 && y >= Top &&
            y < Top + m_Count * m_RowHeight ? (y - Top) / m_RowHeight : -1;
        m_CursorX = x;
        m_CursorY = y;
    }
    void OnMousePressed(vgui::MouseCode code) OVERRIDE
    {
        if (code == MOUSE_LEFT && m_CursorY < 48 && m_CursorX > GetWide() - 54) { Close(); return; }
        if (m_Hover < 0) return;
        const int direction = code == MOUSE_RIGHT || m_CursorX < GetWide() / 3 + 30 ? -1 : 1;
        if (code == MOUSE_LEFT || code == MOUSE_RIGHT) Choose(m_Hover, direction);
    }
    void OnMouseWheeled(int delta) OVERRIDE { if (m_Hover >= 0) Choose(m_Hover, delta > 0 ? -1 : 1); }
    void OnKeyCodePressed(vgui::KeyCode key) OVERRIDE
    {
        if (key == KEY_ESCAPE || key == KEY_C) { Close(); return; }
        if (!m_Count) return;
        if (key == KEY_UP) m_KeyboardSlot = (m_KeyboardSlot + m_Count - 1) % m_Count;
        if (key == KEY_DOWN) m_KeyboardSlot = (m_KeyboardSlot + 1) % m_Count;
        if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_ENTER)
            Choose(m_KeyboardSlot, key == KEY_LEFT ? -1 : 1);
        m_Hover = m_KeyboardSlot;
    }
    void OnThink() OVERRIDE
    {
        BaseClass::OnThink();
        C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
        if (!player || !player->IsAlive() || player->GetActiveWeapon() != m_Weapon.Get()) { Close(); return; }
        if (gpGlobals->curtime >= m_SyncTime)
            for (int i = 0; i < m_Count; ++i) m_Selected[i] = m_Weapon->GetSWEPModuleSelection(i);
    }
private:
    void Choose(int slot, int direction)
    {
        if (!m_Weapon || gpGlobals->curtime < m_NextChange) return;
        const int count = m_Weapon->GetSWEPModuleOptionCount(slot);
        if (count < 2) return;
        m_Selected[slot] = (m_Selected[slot] + direction + count) % count;
        char command[80];
        Q_snprintf(command, sizeof(command), "scp_weapon_module %d %d\n", slot, m_Selected[slot]);
        engine->ServerCmd(command);
        m_NextChange = gpGlobals->curtime + 0.18f;
        m_SyncTime = gpGlobals->curtime + 0.5f;
    }
    void Text(int x, int y, const wchar_t *text, vgui::HFont font, bool dark = false)
    {
        vgui::surface()->DrawSetTextFont(font);
        vgui::surface()->DrawSetTextColor(dark ? Color(10, 10, 10, 255) : Color(245, 245, 245, 255));
        vgui::surface()->DrawSetTextPos(x, y);
        vgui::surface()->DrawPrintText(text, V_wcslen(text));
    }
    void TextUTF8(int x, int y, const char *text, bool dark = false, int maxWidth = 0)
    {
        wchar_t wide[256]; V_UTF8ToUnicode(text, wide, sizeof(wide));
        if (maxWidth > 0)
        {
            int width, height;
            for (int n = V_wcslen(wide); n > 0; --n)
            {
                vgui::surface()->GetTextSize(m_Font, wide, width, height);
                if (width <= maxWidth) break;
                wide[n - 1] = 0;
            }
        }
        Text(x, y, wide, m_Font, dark);
    }
    enum { Top = 86 };
    int m_RowHeight = 46;
    CHandle<CHL2MPScriptedWeapon> m_Weapon;
    vgui::HFont m_Font = 0, m_TitleFont = 0;
    int m_Selected[SCP_WEAPON_MODULE_SLOTS] = {};
    int m_Count = 0, m_Hover = -1, m_KeyboardSlot = 0, m_CursorX = 0, m_CursorY = 0;
    float m_NextChange = 0, m_SyncTime = 0;
};

static vgui::DHANDLE<CWeaponModuleMenu> g_WeaponModuleMenu;
bool SCPWeaponModuleMenuVisible() { return g_WeaponModuleMenu.Get() && g_WeaponModuleMenu->IsVisible(); }

CON_COMMAND(scp_weapon_modules, "Open attachments for the held weapon")
{
    if (SCPWeaponModuleMenuVisible()) { g_WeaponModuleMenu->Close(); return; }
    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if (!player || !player->IsAlive() || engine->IsLevelMainMenuBackground()) return;
    CHL2MPScriptedWeapon *weapon = dynamic_cast<CHL2MPScriptedWeapon *>(player->GetActiveWeapon());
    if (weapon && weapon->IsNativeSWEP()) g_WeaponModuleMenu = new CWeaponModuleMenu(weapon);
}
