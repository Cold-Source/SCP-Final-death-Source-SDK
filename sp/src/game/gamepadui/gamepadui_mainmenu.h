#ifndef GAMEPADUI_MAINMENU_H
#define GAMEPADUI_MAINMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "gamepadui_frame.h"
#include "gamepadui_button.h"
#include "gamepadui_image.h"

namespace GamepadUIMenuStates
{
    enum GamepadUIMenuState
    {
        InGame,
        MainMenu,

        Count
    };
}
using GamepadUIMenuState = GamepadUIMenuStates::GamepadUIMenuState;

class GamepadUIMainMenu : public GamepadUIFrame
{
    DECLARE_CLASS_SIMPLE( GamepadUIMainMenu, GamepadUIFrame );

public:
    GamepadUIMainMenu( vgui::Panel* pParent );

    void ApplySchemeSettings( vgui::IScheme* pScheme ) OVERRIDE;
    void OnCommand( char const* pCommand ) OVERRIDE;
    void OnSetFocus() OVERRIDE;
    void UpdateGradients() OVERRIDE;

    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;

    void LoadMenuButtons();
    void RefreshMenuLanguage();
    void LayoutMainMenu();
    void PaintLogo();
    void OnMenuStateChanged();

    void OnKeyCodeReleased( vgui::KeyCode code );

private:

    void UpdateButtonVisibility();
    GamepadUIMenuState GetCurrentMenuState() const;
    CUtlVector<GamepadUIButton*>& GetCurrentButtons();
    float GetCurrentButtonOffset();
    float GetCurrentLogoOffset();
    int GetNewsLeft();

    CUtlVector<GamepadUIButton*> m_Buttons[ GamepadUIMenuStates::Count ];

    GamepadUIString m_LogoText[ 2 ];
    GamepadUIImage  m_LogoImage;
    GamepadUIButton* m_pQuitButton = nullptr;
    GamepadUIButton* m_pModeButton = nullptr;
    GamepadUIButton* m_pLanguageButton = nullptr;

    GAMEPADUI_PANEL_PROPERTY( float, m_flNewsWidth, "News.Width", "240", SchemeValueTypes::Float );
    GAMEPADUI_PANEL_PROPERTY( Color, m_colNewsBackground, "News.Background", "16 16 16 230", SchemeValueTypes::Color );
    GAMEPADUI_PANEL_PROPERTY( Color, m_colNewsBorder, "News.Border", "255 255 255 80", SchemeValueTypes::Color );
    GAMEPADUI_PANEL_PROPERTY( float, m_flQuitBottomMargin, "Quit.BottomMargin", "18", SchemeValueTypes::Float );
    GAMEPADUI_PANEL_PROPERTY( float, m_flQuitShiftLeft, "Quit.ShiftLeft", "40", SchemeValueTypes::Float );
    GAMEPADUI_PANEL_PROPERTY( float, m_flQuitStackReserve, "Quit.StackReserve", "36", SchemeValueTypes::ProportionalFloat );

    GAMEPADUI_PANEL_PROPERTY( float, m_flButtonSpacing,  "Buttons.Space",    "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flButtonsOffsetX, "Buttons.OffsetX",  "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flButtonsOffsetYMenu,   "Buttons.OffsetY.MainMenu",  "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flButtonsOffsetYInGame, "Buttons.OffsetY.InGame",    "0", SchemeValueTypes::ProportionalFloat );

    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoOffsetX,       "Logo.OffsetX",          "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoOffsetYMenu,   "Logo.OffsetY.MainMenu", "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoOffsetYInGame, "Logo.OffsetY.InGame",   "0", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoImageWidth, "Logo.Image.Width", "480", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoImageGap, "Logo.Image.Gap", "24", SchemeValueTypes::ProportionalFloat );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoImageUVWidth, "Logo.Image.UVWidth", "1", SchemeValueTypes::Float );
    GAMEPADUI_PANEL_PROPERTY( float, m_flLogoImageUVHeight, "Logo.Image.UVHeight", "1", SchemeValueTypes::Float );

    GAMEPADUI_PANEL_PROPERTY( Color, m_colLogoColor, "Logo", "255 255 255 255", SchemeValueTypes::Color );

    vgui::HFont m_hLogoFont;
};

#endif
