#include "gamepadui_interface.h"
#include "gamepadui_basepanel.h"
#include "gamepadui_frame.h"
#include "gamepadui_button.h"
#include "gamepadui_mode.h"
#include "gamepadui_language.h"
#include "gamepadui_util.h"
#include "../shared/scp_gamemode_config.h"
#include "filesystem.h"
#include "vgui/ISurface.h"
#include "tier0/memdbgon.h"

static ConVar scp_ui_coop("scp_ui_coop","0",FCVAR_ARCHIVE,"Legacy preference; use scp_ui_gamemode.");
static ConVar scp_ui_gamemode( "scp_ui_gamemode", "", FCVAR_ARCHIVE, "Selected Lua gamemode folder." );
namespace {
CUtlVector<KeyValues*> s_Modes;
int s_ModeRevision=0;
bool s_Loaded=false;
KeyValues *FindMode(const char *id) {
    FOR_EACH_VEC(s_Modes,i) if(!V_strcmp(s_Modes[i]->GetString("id"),id)) return s_Modes[i];
    return nullptr;
}
void ReloadModes() {
    FOR_EACH_VEC(s_Modes,i) s_Modes[i]->deleteThis();
    s_Modes.RemoveAll(); s_Loaded=true;
    FileFindHandle_t find;
    for(const char *id=g_pFullFileSystem->FindFirstEx("gamemodes/*","MOD",&find);id;id=g_pFullFileSystem->FindNext(find)) {
        if(!g_pFullFileSystem->FindIsDirectory(find) || !SCPGamemodeIdentifier(id) || FindMode(id)) continue;
        KeyValues *mode=SCPReadGamemodeConfig(g_pFullFileSystem,id);
        if(mode) s_Modes.AddToTail(mode);
    }
    g_pFullFileSystem->FindClose(find);
    s_Modes.Sort([](KeyValues *const *a,KeyValues *const *b) {
        int order=(*a)->GetInt("order",100)-(*b)->GetInt("order",100);
        return order ? order : V_strcmp((*a)->GetString("id"),(*b)->GetString("id"));
    });
    if(!FindMode(scp_ui_gamemode.GetString())) {
        ConVarRef legacy("scp_ui_coop");
        KeyValues *preferred=FindMode(legacy.IsValid() && legacy.GetBool() ? "coop" : "campaign");
        scp_ui_gamemode.SetValue(preferred ? preferred->GetString("id") : s_Modes.Count() ? s_Modes[0]->GetString("id") : "");
    }
    ++s_ModeRevision;
}
void EnsureModes() { if(!s_Loaded) ReloadModes(); }
}
void GamepadUIModesShutdown() {
    FOR_EACH_VEC(s_Modes,i) s_Modes[i]->deleteThis();
    s_Modes.RemoveAll(); s_Loaded=false;
}
KeyValues *GamepadUIGetModeConfig() {
    EnsureModes();
    if(GamepadUI::GetInstance().IsInLevel()) {
        ConVarRef mode("gamemode");
        if(mode.IsValid()) { KeyValues *active=FindMode(mode.GetString()); if(active) return active; }
    }
    KeyValues *selected=FindMode(scp_ui_gamemode.GetString());
    // Keep menus usable when all folders were removed; starting a session is disabled.
    static KeyValues *empty=new KeyValues("NoGamemodes");
    return selected ? selected : empty;
}
const char *GamepadUIGetModeId() { return GamepadUIGetModeConfig()->GetString("id"); }
const char *GamepadUIGetModeName() { return SCPGamemodeText(GamepadUIGetModeConfig(),"name",!V_stricmp(GamepadUIGetLanguageCode(),"RU")); }
int GamepadUIModeRevision() { EnsureModes(); return s_ModeRevision; }
bool GamepadUIIsCoopMode() {
    if(GamepadUI::GetInstance().IsInLevel()) { ConVarRef coop("spmp_coop"); return coop.IsValid() && coop.GetBool(); }
    return GamepadUIGetModeConfig()->GetBool("coop");
}
void GamepadUISetCoopMode(bool coop) {
    if(GamepadUIIsCoopMode()==coop) return;
    EnsureModes(); const char *id=coop ? "coop" : "campaign";
    if(FindMode(id)) { scp_ui_gamemode.SetValue(id); ++s_ModeRevision; }
}
namespace
{
char s_SessionCommands[1024] = "";
double s_DisconnectedSince = 0;
}

void GamepadUIStartSession( const char *commands )
{
    V_strncpy( s_SessionCommands, commands, sizeof( s_SessionCommands ) );
    s_DisconnectedSince = 0;
    GamepadUI::GetInstance().GetEngineClient()->ClientCmd_Unrestricted( "disconnect\n" );
}

void GamepadUISessionUpdate()
{
    if ( !s_SessionCommands[0] ) return;
    IVEngineClient *engine = GamepadUI::GetInstance().GetEngineClient();
    // Disconnect changes the host state asynchronously. Wait for shutdown before
    // changing maxplayers or loading an SP save from a multiplayer session.
    if ( engine->IsConnected() || engine->IsInGame() ) { s_DisconnectedSince = 0; return; }
    if ( !s_DisconnectedSince ) s_DisconnectedSince = Plat_FloatTime();
    if ( Plat_FloatTime() - s_DisconnectedSince < 0.1 ) return;
    char command[sizeof( s_SessionCommands )];
    V_strncpy( command, s_SessionCommands, sizeof( command ) );
    s_SessionCommands[0] = 0;
    engine->ClientCmd_Unrestricted( command );
}


namespace {
class ModeButton : public GamepadUIActionButton {
    DECLARE_CLASS_SIMPLE(ModeButton,GamepadUIActionButton);
public:
    ModeButton(vgui::Panel *parent,KeyValues *mode) : BaseClass(parent,"Gamemode",SCPGamemodeText(mode,"name",!V_stricmp(GamepadUIGetLanguageCode(),"RU")),"" ) {
        m_Target=parent; V_strncpy(m_Id,mode->GetString("id"),sizeof(m_Id));
        char command[96]; V_snprintf(command,sizeof(command),"select_mode %s",m_Id); SetCommand(command);
    }
    void OnThink() OVERRIDE {
        SetHighlighted(!V_strcmp(GamepadUIGetModeId(),m_Id)); BaseClass::OnThink();
        if(HasFocus() && !m_HadFocus) { char command[96]; V_snprintf(command,sizeof(command),"reveal_mode %s",m_Id); m_Target->OnCommand(command); }
        m_HadFocus=HasFocus();
    }
    void OnKeyCodePressed(vgui::KeyCode key) OVERRIDE {
        if(key==KEY_ESCAPE || key==KEY_XBUTTON_B) m_Target->OnCommand("action_back"); else BaseClass::OnKeyCodePressed(key);
    }
    void OnMouseWheeled(int delta) OVERRIDE { m_Target->OnMouseWheeled(delta); }
private: char m_Id[64]; vgui::Panel *m_Target; bool m_HadFocus=false;
};
class GamepadUIModePanel : public GamepadUIFrame {
    DECLARE_CLASS_SIMPLE(GamepadUIModePanel,GamepadUIFrame);
public:
    GamepadUIModePanel() : BaseClass(GamepadUI::GetInstance().GetBasePanel(),"ModeSelection") {
        ReloadModes(); SetScheme(vgui::scheme()->LoadSchemeFromFile(GAMEPADUI_RESOURCE_FOLDER "schememode.res","SchemeMode"));
        SetFooterButtons(FooterButtons::Back);
        m_Viewport=new vgui::Panel(this,"ModesViewport"); m_Viewport->SetPaintBackgroundEnabled(false); m_Viewport->SetPaintBorderEnabled(false);
        FOR_EACH_VEC(s_Modes,i) {
            ModeButton *button=new ModeButton(this,s_Modes[i]); button->SetParent(m_Viewport);
            m_Buttons.AddToTail(button);
            if(i) { button->SetNavUp(m_Buttons[i-1]); m_Buttons[i-1]->SetNavDown(button); }
        }
        Activate(); LayoutModes(); UpdateGradients();
        FOR_EACH_VEC(s_Modes,i) if(!V_strcmp(s_Modes[i]->GetString("id"),GamepadUIGetModeId())) { m_Buttons[i]->NavigateTo(); break; }
    }
    void OnThink() OVERRIDE { BaseClass::OnThink(); LayoutModes(); }
    void OnMouseWheeled(int delta) OVERRIDE { m_Scroll=Max(0,m_Scroll-delta*36); LayoutModes(); }
    void Paint() OVERRIDE {
        PaintBackgroundGradients();
        const GamepadUIString title("#ColdUI_Mode");
        vgui::surface()->DrawSetTextFont(m_hTitleFont); vgui::surface()->DrawSetTextColor(255,255,255,255);
        vgui::surface()->DrawSetTextPos(m_X,m_Top/2); vgui::surface()->DrawPrintText(title.String(),title.Length());
        const GamepadUIString description(SCPGamemodeText(GamepadUIGetModeConfig(),"description",!V_stricmp(GamepadUIGetLanguageCode(),"RU")));
        vgui::surface()->DrawSetTextFont(m_hGenericFont); vgui::surface()->DrawSetTextColor(180,180,180,255);
        DrawPrintWrappedText(m_hGenericFont,m_X,GetTall()-m_Top,description.String(),description.Length(),GetWide()-2*m_X,true);
    }
    void OnCommand(const char *command) OVERRIDE {
        if(!V_strncmp(command,"reveal_mode ",12)) {
            const int height=Max(36,GetTall()/14),gap=Max(6,height/5),available=GetTall()-2*m_Top;
            FOR_EACH_VEC(s_Modes,i) if(!V_strcmp(s_Modes[i]->GetString("id"),command+12)) {
                const int top=i*(height+gap); if(top<m_Scroll) m_Scroll=top;
                if(top+height>m_Scroll+available) m_Scroll=top+height-available;
            }
            LayoutModes();
        } else if(!V_strncmp(command,"select_mode ",12)) {
            if(!GamepadUI::GetInstance().IsInLevel() && FindMode(command+12)) {
                scp_ui_gamemode.SetValue(command+12); ++s_ModeRevision; Close();
            }
        } else if(!V_strcmp(command,"action_back")) Close(); else BaseClass::OnCommand(command);
    }
private:
    void LayoutModes() {
        m_X=Max(24,GetWide()/16); m_Top=Max(60,GetTall()/5);
        const int height=Max(36,GetTall()/14),gap=Max(6,height/5),available=GetTall()-2*m_Top;
        m_Viewport->SetBounds(m_X,m_Top,GetWide()-2*m_X,available);
        m_Scroll=clamp(m_Scroll,0,Max(0,m_Buttons.Count()*(height+gap)-available));
        FOR_EACH_VEC(m_Buttons,i) m_Buttons[i]->SetBounds(0,i*(height+gap)-m_Scroll,Min(GetWide()-2*m_X,GetWide()/2),height);
    }
    vgui::Panel *m_Viewport; CUtlVector<ModeButton*> m_Buttons; int m_X=24,m_Top=80,m_Scroll=0;
};
vgui::DHANDLE<GamepadUIModePanel> s_ModePanel;
}
void GamepadUIOpenModeDialog() {
    if(s_ModePanel.Get()) s_ModePanel->Activate(); else s_ModePanel=new GamepadUIModePanel();
}
CON_COMMAND(gamepadui_openmodedialog,"Open the Lua game mode selector.") { GamepadUIOpenModeDialog(); }
