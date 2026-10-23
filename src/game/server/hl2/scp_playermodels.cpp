#include "cbase.h"
#include "scp_playermodels.h"
#include "../../shared/scp_playermodels.h"
#include "engine/ivmodelinfo.h"
#include "tier0/memdbgon.h"

static CUtlVector<SCPPlayerModel> s_PlayerModels;

void SCPPrecachePlayerModels()
{
    SCPLoadPlayerModels( filesystem, s_PlayerModels );
    FOR_EACH_VEC_BACK( s_PlayerModels, i )
    {
        const int index = CBaseEntity::PrecacheModel( s_PlayerModels[i].path );
        const model_t *model = index > 0 ? modelinfo->GetModel( index ) : NULL;
        studiohdr_t *header = model ? modelinfo->GetStudiomodel( model ) : NULL;
        // A listed prop or a broken MDL must never replace a player.
        bool humanoid = false;
        if ( header )
            for ( int bone = 0; bone < header->numbones; ++bone )
                if ( !V_stricmp( header->pBone( bone )->pszName(), "ValveBiped.Bip01_Pelvis" ) )
                    humanoid = true;
        if ( !humanoid )
        {
            Warning( "SCP: ignoring invalid player model %s\n", s_PlayerModels[i].path );
            s_PlayerModels.Remove( i );
        }
    }
}

void SCPApplyPlayerModel( CBasePlayer *player )
{
    ConVarRef cheats( "sv_cheats" );
    if ( !player || !player->edict() || !cheats.IsValid() || !cheats.GetBool() )
        return;
    const int client = engine->IndexOfEdict( player->edict() );
    const char *requested = engine->GetClientConVarValue( client, "cl_scp_playermodel" );
    if ( !SCPValidModelPath( requested ) ) return;
    FOR_EACH_VEC( s_PlayerModels, i )
    {
        if ( V_stricmp( requested, s_PlayerModels[i].path ) ) continue;
        // Only models precached at level initialization are eligible here.
        if ( V_stricmp( STRING( player->GetModelName() ), requested ) )
        {
            player->SetModel( s_PlayerModels[i].path );
            player->SetBodygroup( 0, 0 );
            player->ResetSequence( 0 );
            player->SetCycle( 0 );
        }
        const studiohdr_t *header = player->GetModelPtr() ? player->GetModelPtr()->GetRenderHdr() : NULL;
        const char *skinText = engine->GetClientConVarValue( client, "cl_scp_playerskin" );
        const int skin = skinText ? atoi( skinText ) : 0;
        player->SetSkin( header ? clamp( skin, 0, Max( 0, header->numskinfamilies - 1 ) ) : 0 );
        return;
    }
}
