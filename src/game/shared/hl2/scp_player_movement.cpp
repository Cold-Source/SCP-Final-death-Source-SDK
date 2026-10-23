#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_basehlplayer.h"
#define CHL2_Player C_BaseHLPlayer
#else
#include "hl2_player.h"
#include "globalstate.h"
#endif
#include "in_buttons.h"
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
ConVar hl2_walkspeed( "hl2_walkspeed", "150", FCVAR_REPLICATED );
ConVar hl2_normspeed( "hl2_normspeed", "190", FCVAR_REPLICATED );
ConVar hl2_sprintspeed( "hl2_sprintspeed", "320", FCVAR_REPLICATED );
ConVar sv_infinite_aux_power( "sv_infinite_aux_power", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar sv_stickysprint( "sv_stickysprint", "0", FCVAR_REPLICATED );

bool C_BaseHLPlayer::CanSprint()
{
    return m_bSprintAllowed && !( m_Local.m_bDucked && !m_Local.m_bDucking ) && GetWaterLevel() != 3;
}
#else
extern ConVar hl2_walkspeed, hl2_normspeed, hl2_sprintspeed;
extern CSuitPowerDevice SuitDeviceSprint;
#endif
extern ConVar sv_maxspeed;

// Identical speed selection runs during local prediction and server simulation.
void CHL2_Player::HandleSpeedChanges( CMoveData *mv )
{
#ifndef CLIENT_DLL
    m_bSprintAllowed = m_bSprintEnabled && GlobalEntity_GetState( "suit_no_sprint" ) != GLOBAL_ON;
#endif
    const int changed = mv->m_nButtons ^ mv->m_nOldButtons;
    const bool wantSprint = CanSprint() && ( mv->m_nButtons & IN_SPEED );
    bool sprinting = m_HL2Local.m_bNewSprinting;
    if ( ( changed & IN_SPEED ) && sprinting != wantSprint )
        sprinting = wantSprint && m_HL2Local.m_flSuitPower >= 10.0f;
    if ( !wantSprint || m_HL2Local.m_flSuitPower < 0.01f ) sprinting = false;
    const bool walking = ( mv->m_nButtons & IN_WALK ) && !sprinting && !( mv->m_nButtons & IN_DUCK );
    m_HL2Local.m_bNewSprinting = sprinting;
    m_fIsSprinting = sprinting;
    mv->m_flClientMaxSpeed = sprinting ? hl2_sprintspeed.GetFloat() : walking ? hl2_walkspeed.GetFloat() : hl2_normspeed.GetFloat();
    mv->m_flMaxSpeed = sv_maxspeed.GetFloat();
}

void CHL2_Player::ReduceTimers( CMoveData *mv )
{
    const bool sprinting = m_HL2Local.m_bNewSprinting;
#ifndef CLIENT_DLL
    if ( sprinting ) SuitPower_AddDevice( SuitDeviceSprint );
    else SuitPower_RemoveDevice( SuitDeviceSprint );
    SuitPower_Update();
#else
    // Rewindable local fields already belong to DT_HL2Local's prediction table.
    // Predict sprint drain as well as speed, including stopping at empty energy.
    const float drain = 12.5f;
    const bool wasSprinting = ( m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_SPRINT ) != 0;
    if ( sprinting && !wasSprinting )
    {
        m_HL2Local.m_bitsActiveDevices |= bits_SUIT_DEVICE_SPRINT;
        m_HL2Local.m_flSuitPowerLoad += drain;
    }
    else if ( !sprinting && wasSprinting )
    {
        if ( !sv_infinite_aux_power.GetBool() ) m_HL2Local.m_flSuitPower = Max( 0.0f, m_HL2Local.m_flSuitPower - drain * 0.1f );
        m_HL2Local.m_bitsActiveDevices &= ~bits_SUIT_DEVICE_SPRINT;
        m_HL2Local.m_flSuitPowerLoad -= drain;
        if ( !m_HL2Local.m_bitsActiveDevices ) m_HL2Local.m_flTimeAllSuitDevicesOff = gpGlobals->curtime;
    }
    if ( !m_HL2Local.m_bitsActiveDevices && m_HL2Local.m_flSuitPower < 100 && gpGlobals->curtime >= m_HL2Local.m_flTimeAllSuitDevicesOff + 0.5f )
        m_HL2Local.m_flSuitPower = Min( 100.0f, m_HL2Local.m_flSuitPower + 12.5f * gpGlobals->frametime );
    else if ( m_HL2Local.m_bitsActiveDevices && !sv_infinite_aux_power.GetBool() )
    {
        float load = m_HL2Local.m_flSuitPowerLoad;
        if ( !sv_stickysprint.GetBool() && sprinting && CloseEnough( fabs( GetAbsVelocity().x ), 0.0f ) && CloseEnough( fabs( GetAbsVelocity().y ), 0.0f ) )
            load = CloseEnough( m_HL2Local.m_flSuitPowerLoad, drain ) ? 0.0f : load - drain;
        if ( m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_FLASHLIGHT )
            load -=
#ifdef HL2_EPISODIC
                1.111f
#else
                2.222f
#endif
                * ( 1.0f - 1.0f / Max( 0.001f, m_flFlashlightPowerDrainScale ) );
        m_HL2Local.m_flSuitPower = Max( 0.0f, m_HL2Local.m_flSuitPower - load * gpGlobals->frametime );
    }
#endif
}
