#ifndef GAMEPADUI_NEWS_H
#define GAMEPADUI_NEWS_H

namespace vgui { class Panel; }
vgui::Panel* GamepadUICreateNewsPanel( vgui::Panel* parent );
void GamepadUINewsUpdate();
void GamepadUINewsShutdown();

#endif
