#ifdef _WIN32
#include <windows.h>
#undef CreateFont
#endif
#include "cbase.h"
#include "hud_weaponselector.h"
#include "hud.h"
#include "clientmode.h"
#include "c_baseplayer.h"
#include "c_basecombatweapon.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>
#include <string>

static ConVar hud_weaponselector_switch_delay("hud_weaponselector_switch_delay", "1.00", FCVAR_ARCHIVE,
    "Minimum delay in seconds between custom HUD weapon switches.", true, 0.0f, true, 1.0f);

namespace
{
#ifdef _WIN32
    // Rasterize and crop the visible glyph, excluding the font's empty line spacing.
    // The resulting texture can fill any slot without depending on text scaling.
    bool RasterizeWeaponGlyph(const char* family, wchar_t glyph,
        std::vector<unsigned char>& rgba, int& width, int& height)
    {
        HDC dc = CreateCompatibleDC(NULL);
        HFONT font = CreateFontA(-192, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
            DEFAULT_PITCH, family);
        if (!dc || !font)
        {
            if (font) DeleteObject(font);
            if (dc) DeleteDC(dc);
            return false;
        }
        HGDIOBJ oldFont = SelectObject(dc, font);
        SIZE extent = {};
        ABC abc = {};
        GetTextExtentPoint32W(dc, &glyph, 1, &extent);
        GetCharABCWidthsW(dc, glyph, glyph, &abc);
        const int offset = 2 + MAX(0, -abc.abcA);
        const int canvasWidth = MAX(1, extent.cx + offset + MAX(0, -abc.abcC) + 2);
        const int canvasHeight = MAX(1, extent.cy + 4);
        BITMAPINFO bitmapInfo = {};
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = canvasWidth;
        bitmapInfo.bmiHeader.biHeight = -canvasHeight;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;
        void* pixels = NULL;
        HBITMAP bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &pixels, NULL, 0);
        if (!bitmap || !pixels)
        {
            if (bitmap) DeleteObject(bitmap);
            SelectObject(dc, oldFont);
            DeleteObject(font);
            DeleteDC(dc);
            return false;
        }
        HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
        memset(pixels, 0, canvasWidth * canvasHeight * 4);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        TextOutW(dc, offset, 2, &glyph, 1);
        GdiFlush();
        const unsigned char* source = static_cast<const unsigned char*>(pixels);
        int left = canvasWidth, top = canvasHeight, right = -1, bottom = -1;
        for (int y = 0; y < canvasHeight; ++y)
        {
            for (int x = 0; x < canvasWidth; ++x)
            {
                const unsigned char* pixel = source + (y * canvasWidth + x) * 4;
                if (!pixel[0] && !pixel[1] && !pixel[2])
                    continue;
                left = MIN(left, x); right = MAX(right, x);
                top = MIN(top, y); bottom = MAX(bottom, y);
            }
        }
        const bool valid = right >= left && bottom >= top;
        if (valid)
        {
            width = right - left + 1;
            height = bottom - top + 1;
            rgba.resize(width * height * 4);
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const unsigned char* pixel = source + ((top + y) * canvasWidth + left + x) * 4;
                    unsigned char* target = &rgba[(y * width + x) * 4];
                    target[0] = target[1] = target[2] = 255;
                    target[3] = MAX(pixel[0], MAX(pixel[1], pixel[2]));
                }
            }
        }
        SelectObject(dc, oldBitmap);
        SelectObject(dc, oldFont);
        DeleteObject(bitmap);
        DeleteObject(font);
        DeleteDC(dc);
        return valid;
    }
#endif

    std::vector<std::wstring> WrapSlotText(vgui::HFont font, const wchar_t* text, int width)
    {
        std::vector<std::wstring> lines;
        if (!text || width <= 0)
            return lines;
        const wchar_t* cursor = text;
        while (*cursor)
        {
            while (*cursor == L' ' || *cursor == L'\t' || *cursor == L'\r')
                ++cursor;
            if (!*cursor)
                break;
            if (*cursor == L'\n')
            {
                lines.push_back(L"");
                ++cursor;
                continue;
            }
            const wchar_t* start = cursor;
            const wchar_t* end = start;
            const wchar_t* lastBreak = NULL;
            while (*end && *end != L'\n')
            {
                const std::wstring candidate(start, end + 1);
                int measuredWidth, measuredHeight;
                vgui::surface()->GetTextSize(font, candidate.c_str(), measuredWidth, measuredHeight);
                if (measuredWidth > width)
                    break;
                if (*end == L' ' || *end == L'\t')
                    lastBreak = end;
                ++end;
            }
            // A glyph wider than the entire box cannot be drawn inside it.
            if (end == start)
            {
                ++cursor;
                continue;
            }
            if (*end && *end != L'\n' && *end != L' ' && *end != L'\t' && lastBreak)
                end = lastBreak;
            std::wstring line(start, end);
            while (!line.empty() && (line.back() == L' ' || line.back() == L'\t' || line.back() == L'\r'))
                line.pop_back();
            lines.push_back(line);
            cursor = end;
            if (*cursor == L'\n')
                ++cursor;
        }
        return lines;
    }

    enum WeaponCategory { PRIMARY, LARGE, SIDEARM, UTILITY, CATEGORY_COUNT };

    WeaponCategory GetWeaponCategory(C_BaseCombatWeapon* pWeapon)
    {
        const char* name = pWeapon->GetClassname();
        if (!Q_stricmp(name, "weapon_rpg"))
            return LARGE;
        if (!Q_stricmp(name, "weapon_physcannon") || !Q_stricmp(name, "weapon_physgun"))
            return UTILITY;
        if (pWeapon->GetSlot() == 2 || pWeapon->GetSlot() == 3)
            return PRIMARY;
        if (pWeapon->GetSlot() <= 1)
            return SIDEARM;
        return UTILITY;
    }

    struct SlotLayout
    {
        int x, y, width, height;
    };

    // Match the reference: three upper slots rest on two wide lower slots.
    const int PANEL_WIDTH = 536;
    const int PANEL_HEIGHT = 200;
    const SlotLayout SLOT_LAYOUT[5] =
    {
        { 14, 98, 254, 74 },
        { 268, 98, 254, 74 },
        { 166, 12, 204, 86 },
        { 80, 12, 86, 86 },
        { 370, 12, 86, 86 }
    };
}

// The existing engine commands and client mode look up this HUD name.
DECLARE_NAMED_HUDELEMENT(CHudCustomWeaponSelector, CHudWeaponSelection);

CHudCustomWeaponSelector* GetWeaponSelectorPanel()
{
    return dynamic_cast<CHudCustomWeaponSelector*>(GetHudWeaponSelection());
}

CON_COMMAND(toggle_weapon_selector, "Toggle custom weapon selector")
{
    CHudCustomWeaponSelector* pSelector = GetWeaponSelectorPanel();
    if (pSelector)
        pSelector->ShowSelector(!pSelector->IsInSelectionMode());
}

CHudCustomWeaponSelector::CHudCustomWeaponSelector(const char* pElementName)
    : CBaseHudWeaponSelection(pElementName), BaseClass(NULL, "HudWeaponSelector"), m_iSelectedIndex(-1),
      m_flLayoutScale(0.0f), m_flNextWeaponSwitchTime(0.0f), m_flSwitchCooldownDuration(0.0f),
      m_hTextFont(0)
{
    SetParent(g_pClientMode->GetViewport());
    SetPaintBackgroundEnabled(false);
    SetBounds(0, 0, PANEL_WIDTH, PANEL_HEIGHT);
    // Input comes from gameplay binds, without taking the cursor or keyboard focus.
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
    SetVisible(false);
    m_bSelectionVisible = false;
}

CHudCustomWeaponSelector::~CHudCustomWeaponSelector()
{
    for (const IconTexture& icon : m_IconTextures)
    {
        if (icon.textureId)
            vgui::surface()->DestroyTextureID(icon.textureId);
    }
    if (s_pInstance == this)
        s_pInstance = NULL;
}

bool CHudCustomWeaponSelector::CanSelectWeapons()
{
    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    return pPlayer && pPlayer->IsAlive() && !(pPlayer->GetFlags() & FL_FROZEN)
        && pPlayer->IsAllowedToSwitchWeapons() && !pPlayer->IsInVGuiInputMode()
        && CHudElement::ShouldDraw();
}

void CHudCustomWeaponSelector::Reset()
{
    CBaseHudWeaponSelection::Reset();
    HideSelection();
    m_flNextWeaponSwitchTime = 0.0f;
    m_flSwitchCooldownDuration = 0.0f;
    m_AvailableWeapons.clear();
    RefreshSlotWeapons();
}

void CHudCustomWeaponSelector::LevelShutdown()
{
    HideSelection();
    m_flNextWeaponSwitchTime = 0.0f;
    m_flSwitchCooldownDuration = 0.0f;
    m_AvailableWeapons.clear();
    RefreshSlotWeapons();
}

bool CHudCustomWeaponSelector::ShouldDraw()
{
    if (!IsInSelectionMode())
        return false;
    if (!CanSelectWeapons() || gpGlobals->curtime - m_flSelectionTime > 2.0f)
    {
        HideSelection();
        return false;
    }
    RefreshWeapons();
    if (m_AvailableWeapons.empty())
    {
        HideSelection();
        return false;
    }
    const float fade = clamp((2.0f - (gpGlobals->curtime - m_flSelectionTime)) / 0.35f, 0.0f, 1.0f);
    SetAlpha(static_cast<int>(255.0f * fade));
    return true;
}

void CHudCustomWeaponSelector::ShowSelector(bool bShow)
{
    if (bShow)
        OpenSelection();
    else
        HideSelection();
}

void CHudCustomWeaponSelector::OpenSelection()
{
    if (!CanSelectWeapons())
        return;
    RefreshWeapons();
    if (m_AvailableWeapons.empty())
        return;
    CBaseHudWeaponSelection::OpenSelection();
    UpdateSelectionTime();
    UpdateLayout();
    SetAlpha(255);
    SetVisible(true);
    Repaint();
}

void CHudCustomWeaponSelector::HideSelection()
{
    CBaseHudWeaponSelection::HideSelection();
    m_hSelectedWeapon = NULL;
    m_iSelectedIndex = -1;
    SetVisible(false);
}

void CHudCustomWeaponSelector::RefreshWeapons()
{
    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    C_BaseCombatWeapon* pSelected = m_hSelectedWeapon.Get();
    if (!pSelected && pPlayer)
        pSelected = pPlayer->GetActiveWeapon();

    m_AvailableWeapons.clear();
    if (pPlayer)
    {
        for (int i = 0; i < MAX_WEAPONS; ++i)
        {
            C_BaseCombatWeapon* pWeapon = pPlayer->GetWeapon(i);
            if (!pWeapon || !pWeapon->VisibleInWeaponSelection())
                continue;

            // Group weapons by HUD category, then keep a stable SDK slot/position order.
            auto pos = m_AvailableWeapons.begin();
            for (; pos != m_AvailableWeapons.end(); ++pos)
            {
                C_BaseCombatWeapon* pOther = pos->Get();
                if (GetWeaponCategory(pWeapon) < GetWeaponCategory(pOther)
                    || (GetWeaponCategory(pWeapon) == GetWeaponCategory(pOther)
                        && (pWeapon->GetSlot() < pOther->GetSlot()
                            || (pWeapon->GetSlot() == pOther->GetSlot() && pWeapon->GetPosition() < pOther->GetPosition()))))
                    break;
            }
            m_AvailableWeapons.insert(pos, CHandle<C_BaseCombatWeapon>(pWeapon));
        }
    }

    int selectedIndex = -1;
    int activeIndex = -1;
    for (int i = 0; i < static_cast<int>(m_AvailableWeapons.size()); ++i)
    {
        if (m_AvailableWeapons[i].Get() == pSelected)
            selectedIndex = i;
        if (pPlayer && m_AvailableWeapons[i].Get() == pPlayer->GetActiveWeapon())
            activeIndex = i;
    }
    if (selectedIndex < 0)
        selectedIndex = activeIndex >= 0 ? activeIndex : (m_AvailableWeapons.empty() ? -1 : 0);
    SetSelectedIndex(selectedIndex);
    RefreshSlotWeapons();
}

void CHudCustomWeaponSelector::RefreshSlotWeapons()
{
    const int firstSlot[CATEGORY_COUNT] = { 0, 2, 3, 4 };
    const int capacity[CATEGORY_COUNT] = { 2, 1, 1, 1 };
    int categoryCounts[CATEGORY_COUNT] = {};
    for (int slot = 0; slot < HUD_SLOT_COUNT; ++slot)
        m_SlotWeaponIndices[slot].clear();

    for (int i = 0; i < static_cast<int>(m_AvailableWeapons.size()); ++i)
    {
        C_BaseCombatWeapon* pWeapon = m_AvailableWeapons[i].Get();
        if (!pWeapon)
            continue;
        const int category = GetWeaponCategory(pWeapon);
        const int slot = firstSlot[category] + categoryCounts[category]++ % capacity[category];
        m_SlotWeaponIndices[slot].push_back(i);
    }

    for (int slot = 0; slot < HUD_SLOT_COUNT; ++slot)
    {
        C_BaseCombatWeapon* pPrevious = m_hSlotWeapons[slot].Get();
        m_hSlotWeapons[slot] = NULL;
        for (int index : m_SlotWeaponIndices[slot])
        {
            C_BaseCombatWeapon* pWeapon = m_AvailableWeapons[index].Get();
            if (!m_hSlotWeapons[slot].Get() || pWeapon == pPrevious)
                m_hSlotWeapons[slot] = pWeapon;
        }
        for (int index : m_SlotWeaponIndices[slot])
        {
            if (index == m_iSelectedIndex)
                m_hSlotWeapons[slot] = m_AvailableWeapons[index];
        }
    }
}

void CHudCustomWeaponSelector::SetSelectedIndex(int index)
{
    m_iSelectedIndex = index;
    m_hSelectedWeapon = index >= 0 && index < static_cast<int>(m_AvailableWeapons.size())
        ? m_AvailableWeapons[index].Get() : NULL;
}

C_BaseCombatWeapon* CHudCustomWeaponSelector::GetSelectedWeapon()
{
    return m_hSelectedWeapon.Get();
}

C_BaseCombatWeapon* CHudCustomWeaponSelector::GetWeaponInSlot(int iSlot, int iSlotPos)
{
    C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
    if (!pPlayer)
        return NULL;
    for (int i = 0; i < MAX_WEAPONS; ++i)
    {
        C_BaseCombatWeapon* pWeapon = pPlayer->GetWeapon(i);
        if (pWeapon && pWeapon->GetSlot() == iSlot && pWeapon->GetPosition() == iSlotPos
            && pWeapon->VisibleInWeaponSelection())
            return pWeapon;
    }
    return NULL;
}

bool CHudCustomWeaponSelector::IsSwitchReady() const
{
    return gpGlobals->curtime >= m_flNextWeaponSwitchTime;
}

void CHudCustomWeaponSelector::StartSwitchCooldown()
{
    m_flSwitchCooldownDuration = hud_weaponselector_switch_delay.GetFloat();
    m_flNextWeaponSwitchTime = gpGlobals->curtime + m_flSwitchCooldownDuration;
}

void CHudCustomWeaponSelector::ActivateSelectedWeapon()
{
    if (!IsSwitchReady() || !GetSelectedWeapon() || !GetSelectedWeapon()->CanBeSelected())
        return;
    // Send the actual weapon entity with CUserCmd, rather than a delayed "use" command.
    SetWeaponSelected();
    StartSwitchCooldown();
    OpenSelection();
    C_BasePlayer::GetLocalPlayer()->EmitSound("Player.WeaponSelected");
}

void CHudCustomWeaponSelector::CycleToNextWeapon()
{
    // Selection is controlled by number keys; invnext is intentionally disabled.
}

void CHudCustomWeaponSelector::CycleToPrevWeapon()
{
    // Selection is controlled by number keys; invprev is intentionally disabled.
}

void CHudCustomWeaponSelector::SelectWeaponSlot(int iSlot)
{
    if (!IsSwitchReady() || !CanSelectWeapons())
        return;
    RefreshWeapons();
    const int slot = iSlot - 1;
    if (slot < 0 || slot >= HUD_SLOT_COUNT)
        return;
    const std::vector<int>& indices = m_SlotWeaponIndices[slot];
    const int count = static_cast<int>(indices.size());
    if (!count)
        return;
    int start = 0;
    for (int i = 0; i < count; ++i)
    {
        if (m_AvailableWeapons[indices[i]].Get() == m_hSlotWeapons[slot].Get())
        {
            start = i;
            if (indices[i] == m_iSelectedIndex && count > 1)
                start = (i + 1) % count;
            break;
        }
    }
    // Repeated presses cycle extra weapons in the same square without discarding inventory.
    for (int step = 0; step < count; ++step)
    {
        const int index = indices[(start + step) % count];
        if (m_AvailableWeapons[index].Get() && m_AvailableWeapons[index]->CanBeSelected())
        {
            SetSelectedIndex(index);
            ActivateSelectedWeapon();
            return;
        }
    }
}

void CHudCustomWeaponSelector::SelectWeapon()
{
    // Wheel and number commands already switch immediately, for every hud_fastswitch value.
}

void CHudCustomWeaponSelector::SwitchToLastWeapon()
{
    if (!IsSwitchReady() || !CanSelectWeapons())
        return;
    C_BaseCombatWeapon* pLastWeapon = C_BasePlayer::GetLocalPlayer()->GetLastWeapon();
    if (!pLastWeapon || !pLastWeapon->CanBeSelected())
        return;
    HideSelection();
    CBaseHudWeaponSelection::SwitchToLastWeapon();
    StartSwitchCooldown();
}

void CHudCustomWeaponSelector::ProcessInput()
{
    // This is an immediate-selection HUD: fire and movement remain normal gameplay input.
}

int CHudCustomWeaponSelector::KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding)
{
    if (!CBaseHudWeaponSelection::KeyInput(down, keynum, pszCurrentBinding))
        return 0;
    if (!down)
        return 1;

    // Avoid the old carousel remapping of slot1-slot4 when hud_fastswitch is 3.
    if (pszCurrentBinding)
    {
        for (int slot = 1; slot <= 10; ++slot)
        {
            char binding[16];
            Q_snprintf(binding, sizeof(binding), "slot%d", slot);
            if (!Q_stricmp(pszCurrentBinding, binding) || (slot == 10 && !Q_stricmp(pszCurrentBinding, "slot0")))
            {
                SelectSlot(slot);
                return 0;
            }
        }
    }

    // Support number keys in the open selector, including the numeric keypad.
    if (IsInSelectionMode() && !IsHudMenuTakingInput())
    {
        if (keynum >= KEY_1 && keynum <= KEY_9)
            SelectSlot(keynum - KEY_1 + 1);
        else if (keynum >= KEY_PAD_1 && keynum <= KEY_PAD_9)
            SelectSlot(keynum - KEY_PAD_1 + 1);
        else if (keynum == KEY_0 || keynum == KEY_PAD_0)
            SelectSlot(10);
        else
            return 1;
        return 0;
    }
    return 1;
}

void CHudCustomWeaponSelector::OnThink()
{
    UpdateLayout();
    if (IsInSelectionMode() && !ShouldDraw())
        HideSelection();
}

void CHudCustomWeaponSelector::UpdateLayout()
{
    int screenWide, screenTall;
    vgui::surface()->GetScreenSize(screenWide, screenTall);
    const float scale = MIN(clamp(screenTall / 1080.0f, 0.5f, 1.5f),
        MAX(0.1f, (screenWide - 32.0f) / PANEL_WIDTH));
    const int width = static_cast<int>(PANEL_WIDTH * scale);
    const int height = static_cast<int>(PANEL_HEIGHT * scale);
    const int x = (screenWide - width) / 2;
    const int y = MAX(0, screenTall - height - static_cast<int>(28 * scale));
    int currentX, currentY;
    GetPos(currentX, currentY);
    if (currentX != x || currentY != y || GetWide() != width || GetTall() != height)
        SetBounds(x, y, width, height);

    if (fabsf(m_flLayoutScale - scale) < 0.001f)
        return;
    m_flLayoutScale = scale;
    if (!m_hTextFont)
        m_hTextFont = vgui::surface()->CreateFont();
    const int flags = vgui::ISurface::FONTFLAG_ANTIALIAS | vgui::ISurface::FONTFLAG_DROPSHADOW
        | vgui::ISurface::FONTFLAG_CUSTOM;
    const int textTall = MAX(10, static_cast<int>(13 * scale));
    // Explicit Unicode coverage keeps Cyrillic names in the same face as Latin text.
    // The default range lets VGUI substitute its compatibility font for non-ASCII glyphs.
    if (!vgui::surface()->SetFontGlyphSet(m_hTextFont, "Bahnschrift", textTall, 700, 0, 0,
        flags, 0x0000, 0xFFFF))
        Warning("Weapon selector: could not create Bahnschrift Unicode font.\n");
}

void CHudCustomWeaponSelector::DrawSlotText(vgui::HFont font, const wchar_t* text,
    int x, int y, int width, int height, Color color, bool centered)
{
    if (!text || width <= 0 || height <= 0)
        return;
    const std::vector<std::wstring> lines = WrapSlotText(font, text, width);
    const int fontTall = vgui::surface()->GetFontTall(font);
    const int advance = fontTall + MAX(1, static_cast<int>(2 * m_flLayoutScale));
    const int bottom = y + height;
    vgui::surface()->DrawSetTextFont(font);
    vgui::surface()->DrawSetTextColor(color);
    for (const std::wstring& line : lines)
    {
        int textWide, textTall;
        vgui::surface()->GetTextSize(font, line.c_str(), textWide, textTall);
        if (y + MAX(fontTall, textTall) > bottom)
            break;
        vgui::surface()->DrawSetTextPos(centered ? x + (width - textWide) / 2 : x, y);
        vgui::surface()->DrawPrintText(line.c_str(), static_cast<int>(line.size()));
        y += advance;
    }
}

vgui::HFont CHudCustomWeaponSelector::GetSmallIconFont(vgui::HFont sourceFont, wchar_t glyph,
    int maxWidth, int maxHeight, int& width, int& height)
{
    width = height = 0;
    if (maxWidth <= 0 || maxHeight <= 0)
        return 0;

    const char* family = vgui::surface()->GetFontFamilyName(sourceFont);
    if (!family || !family[0])
        family = "HalfLife2";
    const wchar_t text[2] = { glyph, 0 };
    // TrueType weapon glyphs need an actually smaller font: text scaling is not reliable here.
    const int initialTall = maxHeight;
    for (int tall = initialTall; tall >= 6; --tall)
    {
        vgui::HFont font = 0;
        for (const IconFont& cached : m_IconFonts)
        {
            if (cached.tall == tall && !Q_stricmp(cached.family, family))
            {
                font = cached.font;
                break;
            }
        }
        if (!font)
        {
            font = vgui::surface()->CreateFont();
            const int flags = vgui::ISurface::FONTFLAG_ANTIALIAS | vgui::ISurface::FONTFLAG_CUSTOM;
            if (!vgui::surface()->SetFontGlyphSet(font, family, tall, 0, 0, 0, flags))
                return 0;
            IconFont cached;
            Q_strncpy(cached.family, family, sizeof(cached.family));
            cached.tall = tall;
            cached.font = font;
            m_IconFonts.push_back(cached);
        }
        vgui::surface()->GetTextSize(font, text, width, height);
        if (width > 0 && height > 0 && width <= maxWidth && height <= maxHeight)
            return font;
    }
    width = height = 0;
    return 0;
}

bool CHudCustomWeaponSelector::DrawFontIcon(vgui::HFont sourceFont, wchar_t glyph,
    int x, int y, int width, int height, Color color)
{
#ifdef _WIN32
    const char* family = vgui::surface()->GetFontFamilyName(sourceFont);
    if (!family || !family[0])
        family = "HalfLife2";
    IconTexture* cached = NULL;
    for (IconTexture& icon : m_IconTextures)
    {
        if (icon.glyph == glyph && !Q_stricmp(icon.family, family))
        {
            cached = &icon;
            break;
        }
    }
    if (!cached)
    {
        IconTexture icon = {};
        Q_strncpy(icon.family, family, sizeof(icon.family));
        icon.glyph = glyph;
        std::vector<unsigned char> rgba;
        if (RasterizeWeaponGlyph(family, glyph, rgba, icon.width, icon.height))
        {
            // Upload a power-of-two backing texture with a transparent border.
            // Some Source render paths reject arbitrary cropped glyph dimensions.
            icon.textureWidth = icon.textureHeight = 1;
            while (icon.textureWidth < icon.width + 2) icon.textureWidth *= 2;
            while (icon.textureHeight < icon.height + 2) icon.textureHeight *= 2;
            std::vector<unsigned char> upload(icon.textureWidth * icon.textureHeight * 4, 255);
            for (size_t pixel = 3; pixel < upload.size(); pixel += 4)
                upload[pixel] = 0;
            for (int row = 0; row < icon.height; ++row)
                memcpy(&upload[((row + 1) * icon.textureWidth + 1) * 4],
                    &rgba[row * icon.width * 4], icon.width * 4);
            icon.textureId = vgui::surface()->CreateNewTextureID(true);
            if (icon.textureId > 0)
            {
                vgui::surface()->DrawSetTextureRGBA(icon.textureId, upload.data(),
                    icon.textureWidth, icon.textureHeight, 1, true);
                int uploadedWidth = 0, uploadedHeight = 0;
                vgui::surface()->DrawGetTextureSize(icon.textureId, uploadedWidth, uploadedHeight);
                if (!vgui::surface()->IsTextureIDValid(icon.textureId)
                    || uploadedWidth != icon.textureWidth || uploadedHeight != icon.textureHeight)
                {
                    vgui::surface()->DestroyTextureID(icon.textureId);
                    icon.textureId = 0;
                }
            }
            else
                icon.textureId = 0;
        }
        m_IconTextures.push_back(icon);
        cached = &m_IconTextures.back();
    }
    if (cached->textureId && vgui::surface()->IsTextureIDValid(cached->textureId)
        && width > 0 && height > 0)
    {
        const float scale = MIN(width / static_cast<float>(cached->width), height / static_cast<float>(cached->height));
        const int drawWide = MIN(width, MAX(1, static_cast<int>(cached->width * scale)));
        const int drawTall = MIN(height, MAX(1, static_cast<int>(cached->height * scale)));
        const int left = x + (width - drawWide) / 2;
        const int top = y + (height - drawTall) / 2;
        vgui::surface()->DrawSetColor(color);
        vgui::surface()->DrawSetTexture(cached->textureId);
        vgui::surface()->DrawTexturedSubRect(left, top, left + drawWide, top + drawTall,
            1.0f / cached->textureWidth, 1.0f / cached->textureHeight,
            (cached->width + 1.0f) / cached->textureWidth,
            (cached->height + 1.0f) / cached->textureHeight);
        vgui::surface()->DrawSetTexture(-1);
        return true;
    }
#endif
    // Portable fallback when native glyph rasterization is unavailable.
    int iconWide, iconTall;
    const vgui::HFont font = GetSmallIconFont(sourceFont, glyph, width, height, iconWide, iconTall);
    if (!font)
        return false;
    vgui::surface()->DrawSetTextFont(font);
    vgui::surface()->DrawSetTextColor(color);
    vgui::surface()->DrawSetTextPos(x + (width - iconWide) / 2, y + (height - iconTall) / 2);
    vgui::surface()->DrawUnicodeChar(glyph);
    return true;
}

void CHudCustomWeaponSelector::DrawSlot(int slot, int x, int y, int width, int height)
{
    vgui::surface()->DrawSetTexture(-1);
    C_BaseCombatWeapon* pWeapon = m_hSlotWeapons[slot].Get();
    const bool selected = pWeapon && pWeapon == GetSelectedWeapon();
    const bool selectable = pWeapon && pWeapon->CanBeSelected();
    const int inset = MAX(4, static_cast<int>(6 * m_flLayoutScale));
    const int badge = MAX(14, static_cast<int>(20 * m_flLayoutScale));
    const Color accent(245, 245, 245, 255);
    const Color border = selected ? accent : Color(135, 135, 135, pWeapon ? 185 : 100);
    const Color foreground = selected ? Color(255, 255, 255, 255)
        : Color(210, 210, 210, selectable ? 225 : 105);
    const int shade = selected ? 28 : 14;
    vgui::surface()->DrawSetColor(shade, shade, shade, 240);
    vgui::surface()->DrawFilledRect(x, y, x + width, y + height);
    const int frame = MAX(selected ? 3 : 2, static_cast<int>((selected ? 4 : 3) * m_flLayoutScale));
    vgui::surface()->DrawSetColor(border);
    vgui::surface()->DrawFilledRect(x, y, x + width, y + frame);
    vgui::surface()->DrawFilledRect(x, y + height - frame, x + width, y + height);
    vgui::surface()->DrawFilledRect(x, y + frame, x + frame, y + height - frame);
    vgui::surface()->DrawFilledRect(x + width - frame, y + frame, x + width, y + height - frame);
    if (selected)
    {
        float progress = 1.0f;
        if (m_flSwitchCooldownDuration > 0.0f)
            progress = clamp(1.0f - (m_flNextWeaponSwitchTime - gpGlobals->curtime) / m_flSwitchCooldownDuration, 0.0f, 1.0f);
        vgui::surface()->DrawSetColor(70, 70, 70, 255);
        vgui::surface()->DrawFilledRect(x, y + height - frame, x + width, y + height);
        vgui::surface()->DrawSetColor(accent);
        vgui::surface()->DrawFilledRect(x, y + height - frame, x + static_cast<int>(width * progress), y + height);
    }

    // The icon uses the whole slot interior. Corner labels are drawn on top afterwards.
    const int iconX = x + inset;
    const int iconY = y + inset;
    const int iconWidth = MAX(1, width - inset * 2);
    const int iconHeight = MAX(1, height - inset * 2);
    if (pWeapon)
    {
        const CHudTexture* icon = selected ? pWeapon->GetSpriteActive() : pWeapon->GetSpriteInactive();
        if (!icon)
            icon = pWeapon->GetSpriteInactive();
        if (icon && icon->bRenderUsingFont)
        {
            const wchar_t glyph = static_cast<wchar_t>(static_cast<unsigned char>(icon->cCharacterInFont));
            DrawFontIcon(icon->hFont, glyph, iconX, iconY, iconWidth, iconHeight, foreground);
        }
        else if (icon && icon->Width() > 0 && icon->Height() > 0)
        {
            const float scale = MIN(iconWidth / static_cast<float>(icon->Width()), iconHeight / static_cast<float>(icon->Height()));
            const int drawWide = MIN(iconWidth, MAX(1, static_cast<int>(icon->Width() * scale)));
            const int drawTall = MIN(iconHeight, MAX(1, static_cast<int>(icon->Height() * scale)));
            icon->DrawSelf(iconX + (iconWidth - drawWide) / 2, iconY + (iconHeight - drawTall) / 2,
                drawWide, drawTall, foreground);
        }
    }
    else
    {
        const int centerX = x + width / 2;
        const int centerY = y + height / 2;
        const int half = MAX(3, static_cast<int>(4 * m_flLayoutScale));
        vgui::surface()->DrawSetColor(135, 135, 135, 110);
        vgui::surface()->DrawLine(centerX - half, centerY, centerX + half, centerY);
        vgui::surface()->DrawLine(centerX, centerY - half, centerX, centerY + half);
    }

    vgui::surface()->DrawSetTexture(-1);
    vgui::surface()->DrawSetColor(selected ? Color(245, 245, 245, 240) : Color(32, 32, 32, 240));
    vgui::surface()->DrawFilledRect(x + inset, y + inset, x + inset + badge, y + inset + badge);
    wchar_t number[8];
    V_snwprintf(number, ARRAYSIZE(number), L"%d", slot + 1);
    const int textTall = vgui::surface()->GetFontTall(m_hTextFont);
    DrawSlotText(m_hTextFont, number, x + inset, y + inset + (badge - textTall) / 2,
        badge, textTall, selected ? Color(15, 15, 15, 255) : Color(235, 235, 235, 235), true);

    const int groupedCount = static_cast<int>(m_SlotWeaponIndices[slot].size());
    if (groupedCount > 1)
    {
        int position = 1;
        for (int i = 0; i < groupedCount; ++i)
        {
            if (m_AvailableWeapons[m_SlotWeaponIndices[slot][i]].Get() == pWeapon)
                position = i + 1;
        }
        wchar_t counter[20];
        V_snwprintf(counter, ARRAYSIZE(counter), L"%d/%d", position, groupedCount);
        int counterWide, counterTall;
        vgui::surface()->GetTextSize(m_hTextFont, counter, counterWide, counterTall);
        counterWide = MIN(counterWide, MAX(1, width - inset * 2 - badge - 2));
        const int left = x + width - inset - counterWide;
        vgui::surface()->DrawSetColor(14, 14, 14, 230);
        vgui::surface()->DrawFilledRect(left - 2, y + inset, x + width - inset, y + inset + counterTall);
        DrawSlotText(m_hTextFont, counter, left, y + inset, counterWide, counterTall, foreground, false);
    }
    if (pWeapon)
    {
        const int ammoType = pWeapon->GetPrimaryAmmoType();
        C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
        if (ammoType >= 0 && pPlayer)
        {
            wchar_t ammo[32];
            if (pWeapon->Clip1() >= 0)
                V_snwprintf(ammo, ARRAYSIZE(ammo), L"%d / %d", pWeapon->Clip1(), pPlayer->GetAmmoCount(ammoType));
            else
                V_snwprintf(ammo, ARRAYSIZE(ammo), L"%d", pPlayer->GetAmmoCount(ammoType));
            int ammoWide, ammoTall;
            vgui::surface()->GetTextSize(m_hTextFont, ammo, ammoWide, ammoTall);
            const int ammoWidth = MIN(ammoWide, iconWidth);
            const int lines = static_cast<int>(WrapSlotText(m_hTextFont, ammo, ammoWidth).size());
            const int advance = textTall + MAX(1, static_cast<int>(2 * m_flLayoutScale));
            const int ammoHeight = MIN(iconHeight, textTall + MAX(0, lines - 1) * advance);
            const int left = x + width - inset - ammoWidth;
            const int top = y + height - inset - ammoHeight;
            vgui::surface()->DrawSetColor(14, 14, 14, 230);
            vgui::surface()->DrawFilledRect(left - 2, top, x + width - inset, top + ammoHeight);
            DrawSlotText(m_hTextFont, ammo, left, top, ammoWidth, ammoHeight, foreground, false);
        }
    }
}

void CHudCustomWeaponSelector::Paint()
{
    BaseClass::Paint();
    UpdateLayout();
    RefreshWeapons();
    for (int slot = 0; slot < HUD_SLOT_COUNT; ++slot)
    {
        const SlotLayout& layout = SLOT_LAYOUT[slot];
        DrawSlot(slot, static_cast<int>(layout.x * m_flLayoutScale), static_cast<int>(layout.y * m_flLayoutScale),
            static_cast<int>(layout.width * m_flLayoutScale), static_cast<int>(layout.height * m_flLayoutScale));
    }
    DrawSlotText(m_hTextFont, L"1 - 5", 0,
        GetTall() - vgui::surface()->GetFontTall(m_hTextFont), GetWide(),
        vgui::surface()->GetFontTall(m_hTextFont), Color(170, 170, 170, 220), true);
}
