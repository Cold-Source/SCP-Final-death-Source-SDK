#ifndef GAMEPADUI_MODE_H
#define GAMEPADUI_MODE_H
#include "tier1/KeyValues.h"
void GamepadUIOpenModeDialog();
KeyValues *GamepadUIGetModeConfig();
const char *GamepadUIGetModeId();
const char *GamepadUIGetModeName();
int GamepadUIModeRevision();
bool GamepadUIIsCoopMode();
void GamepadUISetCoopMode( bool coop );
void GamepadUIStartSession( const char *commands );
void GamepadUISessionUpdate();
void GamepadUIModesShutdown();
#endif
