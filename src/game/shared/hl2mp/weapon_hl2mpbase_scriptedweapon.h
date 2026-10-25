//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basecombatweapon_shared.h"
#include "scp_weapon_module.h"
#include "luamanager.h"

#ifndef BASEHLCOMBATWEAPON_H
#define BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
	#define CHL2MPScriptedWeapon C_HL2MPScriptedWeapon
#endif

//=========================================================
// Scripted weapon base class
//=========================================================
class CHL2MPScriptedWeapon : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CHL2MPScriptedWeapon, CBaseCombatWeapon);
	DECLARE_DATADESC();

	CHL2MPScriptedWeapon();
	~CHL2MPScriptedWeapon();

	bool			IsScripted( void ) const { return true; }
    bool IsAiming() const { return m_bSWEPAiming; }
    void SetAimState(bool aiming) { m_bSWEPAiming = aiming; }
    bool IsNativeSWEP() const;
    int GetSWEPModuleCount() const;
    int GetSWEPModuleOptionCount(int slot) const;
    int GetSWEPModuleSelection(int slot) const
    {
        const int count = GetSWEPModuleOptionCount(slot);
        return count > 0 && m_iSWEPModules[slot] >= 0 && m_iSWEPModules[slot] < count ? m_iSWEPModules[slot] : 0;
    }
    void GetSWEPModuleName(int slot, char *buffer, int size) const;
    bool GetSWEPModuleOption(int slot, int option, SCPWeaponModuleOption &out) const;
    void ApplySWEPModules(CBaseAnimating *target, bool world) const;
#ifndef CLIENT_DLL
    bool SetSWEPModule(int slot, int option);
#else
    void DrawSWEPModules(C_BaseAnimating *parent, int flags, bool world);
    float GetSWEPAimFraction() const { return m_flSWEPAimBlend; }
    CHandle<C_BaseAnimating> m_hSWEPViewParts[SCP_WEAPON_MODULE_SLOTS];
    CHandle<C_BaseAnimating> m_hSWEPLensParts[SCP_WEAPON_MODULE_SLOTS];
    CHandle<C_BaseAnimating> m_hSWEPWorldParts[SCP_WEAPON_MODULE_SLOTS];
#endif
    bool IsSafetyOn() const { return m_bSWEPSafety; }
    void SetSafetyState(bool enabled) { m_bSWEPSafety = enabled; }
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	// DECLARE_ACTTABLE();

	acttable_t m_acttable[LUA_MAX_WEAPON_ACTIVITIES];
	acttable_t *ActivityList( void );
	int ActivityListCount( void );
	acttable_t *ActivityList(int &count) OVERRIDE { count = ActivityListCount(); return ActivityList(); }
    Activity ActivityOverride(Activity activity, bool *required) OVERRIDE;

	void			Precache( void );
	void			InitScriptedWeapon( void );

	void	PrimaryAttack( void );
	void	SecondaryAttack( void );

	// Firing animations
	virtual Activity		GetDrawActivity( void );

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void	ItemPostFrame( void );
	virtual void	ItemBusyFrame( void );
	virtual void	FireBullets( const FireBulletsInfo_t &info );
	virtual bool	Reload();
	
	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );

#ifdef CLIENT_DLL
	virtual void	OnDataChanged( DataUpdateType_t updateType );
    void AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles) OVERRIDE;
    float GetSWEPViewModelFOV(float fallback) const;
    void GetAimEntOrigin(IClientEntity *attachedTo, Vector *origin, QAngle *angles) OVERRIDE;
    float m_flSWEPSprintBlend = 0.0f;
    float m_flSWEPAimBlend = 0.0f;
    float m_flSWEPSafetyBlend = 0.0f;
    float m_flSWEPRecoilBlend = 0.0f;
    float m_flSWEPWalkBlend = 0.0f;
    float m_flSWEPStepPhase = 0.0f;
    int m_iSWEPPreviousClip = -2;
    int m_nSWEPViewFrame = -1;
	virtual const char *GetScriptedClassname( void );
#endif

	virtual const Vector &GetBulletSpread( void );

public:

	// Weapon info accessors for data in the weapon's data file
	FileWeaponInfo_t *m_pLuaWeaponInfo;
	virtual const FileWeaponInfo_t	&GetWpnData( void ) const;
	virtual const char		*GetViewModel( int viewmodelindex = 0 ) const;
	virtual const char		*GetWorldModel( void ) const;
	virtual const char		*GetAnimPrefix( void ) const;
	virtual int				GetMaxClip1( void ) const;
	virtual int				GetMaxClip2( void ) const;
	virtual int				GetDefaultClip1( void ) const;
	virtual int				GetDefaultClip2( void ) const;
	virtual int				GetWeight( void ) const;
	virtual bool			AllowsAutoSwitchTo( void ) const;
	virtual bool			AllowsAutoSwitchFrom( void ) const;
	virtual int				GetWeaponFlags( void ) const;
	virtual int				GetSlot( void ) const;
	virtual int				GetPosition( void ) const;
	virtual char const		*GetPrintName( void ) const;
	bool					IsMeleeWeapon() const;

public:
// Server Only Methods
#if !defined( CLIENT_DLL )

	virtual int				CapabilitiesGet( void );

// Client only methods
#else

	// Returns the aiment render origin + angles
	virtual int				DrawModel( int flags );

#endif // End client-only methods

private:
	
	CHL2MPScriptedWeapon( const CHL2MPScriptedWeapon & );

	CNetworkString( m_iScriptedClassname, MAX_WEAPON_STRING );
    CNetworkVar(bool, m_bSWEPAiming);
    CNetworkVar(bool, m_bSWEPSafety);
    CNetworkArray(int, m_iSWEPModules, SCP_WEAPON_MODULE_SLOTS);

};

void RegisterScriptedWeapon( const char *szClassname );
void ResetWeaponFactoryDatabase( void );

#endif // BASEHLCOMBATWEAPON_H
