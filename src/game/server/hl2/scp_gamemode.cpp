#include "cbase.h"
#include "scp_gamemode.h"
#include "../../shared/scp_gamemode_config.h"
#include "filesystem.h"
#include "tier0/memdbgon.h"
namespace {
KeyValues *s_Config=nullptr;

}
void SCPGamemodeConfigureServer() {
    if(s_Config) { s_Config->deleteThis(); s_Config=nullptr; }
    ConVarRef mode("gamemode");
    s_Config=SCPReadGamemodeConfig(filesystem,mode.IsValid() ? mode.GetString() : "campaign");
    if(!s_Config) {
        Warning("Invalid server gamemode; falling back to campaign.\n");
        if(mode.IsValid()) mode.SetValue("campaign");
        s_Config=SCPReadGamemodeConfig(filesystem,"campaign");
    }
    if(!s_Config) s_Config=new KeyValues("UnavailableGamemode");
    ConVarRef coop("spmp_coop"), engineCoop("coop");
    const bool multiplayer=s_Config->GetBool("coop");
    if(coop.IsValid()) coop.SetValue(multiplayer);
    if(engineCoop.IsValid()) engineCoop.SetValue(multiplayer);
    Msg("Lua gamemode: %s (%s rules).\n",s_Config->GetString("id"),multiplayer ? "co-op" : "single-player");
}
