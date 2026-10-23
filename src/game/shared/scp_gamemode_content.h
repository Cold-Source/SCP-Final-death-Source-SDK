#ifndef SCP_GAMEMODE_CONTENT_H
#define SCP_GAMEMODE_CONTENT_H
#include "scp_gamemode_config.h"
#include "filesystem.h"
inline void SCPUnmountGamemodeContent(IFileSystem *fs,char *mounted,bool scripts=true)
{
    if(!mounted[0]) return;
    if(scripts) fs->RemoveSearchPath(mounted,"MOD");
    fs->RemoveSearchPath(mounted,"GAME");
    mounted[0]=0;
}
inline bool SCPMountGamemodeContent(IFileSystem *fs,const char *gameDirectory,const char *id,char *mounted,int capacity,bool scripts=true)
{
    SCPUnmountGamemodeContent(fs,mounted,scripts);
    if(!SCPGamemodeIdentifier(id)) return false;
    char relative[MAX_PATH]; V_snprintf(relative,sizeof(relative),"gamemodes/%s/content",id);
    char full[MAX_PATH]; V_ComposeFileName(gameDirectory,relative,full,sizeof(full)); V_FixSlashes(full);
    if(!fs->IsDirectory(full)) return false;
    V_strncpy(mounted,full,capacity);
    if(scripts) fs->AddSearchPath(mounted,"MOD",PATH_ADD_TO_HEAD);
    fs->AddSearchPath(mounted,"GAME",PATH_ADD_TO_HEAD);
    return true;
}
#endif
