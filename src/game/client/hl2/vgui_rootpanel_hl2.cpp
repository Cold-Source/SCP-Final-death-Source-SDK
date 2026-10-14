//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#ifdef LUA_SDK
#include "vgui/IVgui.h"
#include "hl2/vgui_rootpanel_hl2.h"
#include "clientmode_shared.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef LUA_SDK
C_ScriptedBaseGameUIPanel* g_pScriptedBaseGameUIPanel = NULL;
C_ScriptedBaseINGamePanel* g_pScriptedBaseINGamePanel = NULL;


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateGameUIRootPanel(void)
{
	g_pScriptedBaseGameUIPanel = new C_ScriptedBaseGameUIPanel(enginevgui->GetPanel(PANEL_GAMEUIDLL));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyGameUIRootPanel(void)
{
	delete g_pScriptedBaseGameUIPanel;
	g_pScriptedBaseGameUIPanel = NULL;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateClientDLLRootPanel( void )
{
	// Just using PANEL_ROOT in HL2 right now
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyClientDLLRootPanel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Game specific root panel
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::VPANEL VGui_GetClientDLLRootPanel( void )
{
	vgui::VPANEL root = enginevgui->GetPanel( PANEL_CLIENTDLL );
	return root;
}
#ifdef LUA_SDK
vgui::Panel* VGui_GetGameUIPanel(void)
{
	return g_pScriptedBaseGameUIPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Game specific root panel
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::Panel* VGui_GetClientLuaRootPanel(void)
{
	ClientModeShared* mode = (ClientModeShared*)GetClientModeNormal();
	vgui::Panel* pRoot = mode->m_pClientLuaPanel;
	return pRoot;
}
#endif
#ifdef LUA_SDK
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_CreateINGameRootPanel(void)
{
	g_pScriptedBaseINGamePanel = new C_ScriptedBaseINGamePanel(enginevgui->GetPanel(PANEL_INGAMESCREENS));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGUI_DestroyINGameRootPanel(void)
{
	delete g_pScriptedBaseINGamePanel;
	g_pScriptedBaseINGamePanel = NULL;
}

vgui::Panel* VGui_GetINGamePanel(void)
{
	return g_pScriptedBaseINGamePanel;
}
#endif

#ifdef LUA_SDK
C_ScriptedBaseGameUIPanel::C_ScriptedBaseGameUIPanel(vgui::VPANEL parent)
	: BaseClass(NULL, "ScriptedBaseGameUIPanel")
{
	SetParent(parent);
	SetPaintEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	// This panel does post child painting
	SetPostChildPaintEnabled(true);

	// Make it screen sized
	SetBounds(0, 0, ScreenWidth(), ScreenHeight());

	// Ask for OnTick messages
	vgui::ivgui()->AddTickSignal(GetVPanel());
}

C_ScriptedBaseGameUIPanel::~C_ScriptedBaseGameUIPanel(void)
{
}

void C_ScriptedBaseGameUIPanel::PostChildPaint()
{
	BaseClass::PostChildPaint();

	// Draw all panel effects
	RenderPanelEffects();
}

void C_ScriptedBaseGameUIPanel::RenderPanelEffects(void)
{
}

void C_ScriptedBaseGameUIPanel::OnTick(void)
{
}

void C_ScriptedBaseGameUIPanel::LevelInit(void)
{
}

void C_ScriptedBaseGameUIPanel::LevelShutdown(void)
{
}

C_ScriptedBaseINGamePanel::C_ScriptedBaseINGamePanel(vgui::VPANEL parent)
	: BaseClass(NULL, "ScriptedBaseINGamePanel")
{
	SetParent(parent);
	SetPaintEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	// This panel does post child painting
	SetPostChildPaintEnabled(true);

	// Make it screen sized
	SetBounds(0, 0, ScreenWidth(), ScreenHeight());

	// Ask for OnTick messages
	vgui::ivgui()->AddTickSignal(GetVPanel());
}

C_ScriptedBaseINGamePanel::~C_ScriptedBaseINGamePanel(void)
{
}

void C_ScriptedBaseINGamePanel::PostChildPaint()
{
	BaseClass::PostChildPaint();

	// Draw all panel effects
	RenderPanelEffects();
}

void C_ScriptedBaseINGamePanel::RenderPanelEffects(void)
{
}

void C_ScriptedBaseINGamePanel::OnTick(void)
{
}

void C_ScriptedBaseINGamePanel::LevelInit(void)
{
}

void C_ScriptedBaseINGamePanel::LevelShutdown(void)
{
}
#endif
