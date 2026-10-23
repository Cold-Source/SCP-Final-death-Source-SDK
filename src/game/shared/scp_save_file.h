#ifndef SCP_SAVE_FILE_H
#define SCP_SAVE_FILE_H
class IFileSystem;
class CUtlBuffer;
bool SCPWriteSaveAtomic( IFileSystem *fs, const char *path, CUtlBuffer &buffer );
#endif
