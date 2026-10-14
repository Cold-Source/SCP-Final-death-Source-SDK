#ifndef SPMP_SAVELOAD_H
#define SPMP_SAVELOAD_H
#ifdef _WIN32
#pragma once
#endif

class CBaseEntity;
class CBasePlayer;
bool SpmpIsRestoringWorld();
bool SpmpUsesOwnSaveMemory();
CBasePlayer *SpmpPlayerForSavedSlot( int slot );

#endif
