#pragma once
#include "weapon_selection.h"
#include <Color.h>
#include <vgui_controls/EditablePanel.h>
#include <vector>

class C_BaseCombatWeapon;

class CHudCustomWeaponSelector : public CBaseHudWeaponSelection, public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudCustomWeaponSelector, vgui::EditablePanel);
public:
    CHudCustomWeaponSelector(const char* pElementName);
    ~CHudCustomWeaponSelector() override;

    void ShowSelector(bool bShow);
    bool ShouldDraw() override;
    void Reset() override;
    void LevelShutdown() override;
    void OpenSelection() override;
    void HideSelection() override;
    void CycleToNextWeapon() override;
    void CycleToPrevWeapon() override;
    void SelectWeaponSlot(int iSlot) override;
    void SelectWeapon() override;
    void SwitchToLastWeapon() override;
    void ProcessInput() override;
    int KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) override;
    C_BaseCombatWeapon* GetSelectedWeapon() override;
    C_BaseCombatWeapon* GetWeaponInSlot(int iSlot, int iSlotPos) override;

protected:
    void Paint() override;
    void OnThink() override;
    bool IsWeaponSelectable() override { return false; }

private:
    bool CanSelectWeapons();
    bool IsSwitchReady() const;
    void StartSwitchCooldown();
    void RefreshWeapons();
    void SetSelectedIndex(int index);
    void ActivateSelectedWeapon();
    void RefreshSlotWeapons();
    void UpdateLayout();
    void DrawSlot(int slot, int x, int y, int width, int height);
    void DrawSlotText(vgui::HFont font, const wchar_t* text, int x, int y, int width, int height, Color color, bool centered);
    vgui::HFont GetSmallIconFont(vgui::HFont sourceFont, wchar_t glyph, int maxWidth, int maxHeight, int& width, int& height);
    bool DrawFontIcon(vgui::HFont sourceFont, wchar_t glyph, int x, int y, int width, int height, Color color);

    struct IconTexture
    {
        char family[64];
        wchar_t glyph;
        int textureId;
        int width, height;
        int textureWidth, textureHeight;
    };
    std::vector<IconTexture> m_IconTextures;

    struct IconFont
    {
        char family[64];
        int tall;
        vgui::HFont font;
    };
    std::vector<IconFont> m_IconFonts;

    enum { HUD_SLOT_COUNT = 5 };

    int m_iSelectedIndex;
    std::vector<CHandle<C_BaseCombatWeapon>> m_AvailableWeapons;
    std::vector<int> m_SlotWeaponIndices[HUD_SLOT_COUNT];
    CHandle<C_BaseCombatWeapon> m_hSlotWeapons[HUD_SLOT_COUNT];
    float m_flLayoutScale;
    float m_flNextWeaponSwitchTime;
    float m_flSwitchCooldownDuration;
    vgui::HFont m_hTextFont;
};

CHudCustomWeaponSelector* GetWeaponSelectorPanel();
