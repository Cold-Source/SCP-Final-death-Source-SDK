#ifndef SCP_ADMIN_H
#define SCP_ADMIN_H
#ifdef _WIN32
#pragma once
#endif
class CBasePlayer;
class CCommand;
bool SCPAdminCommand( CBasePlayer *player, const CCommand &args );
#endif
