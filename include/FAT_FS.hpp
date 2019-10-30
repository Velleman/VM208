#ifndef FAT_FS_HPP
#define FAT_FS_HPP
#include "filesystem.hpp"
#include "FFat.h"
class FAT_Fs : public FileSystem
{
private:
public:
    FAT_Fs();
    void begin();
    File open(String path, const char* mode);
    bool exist(String path);
    void remove(String path);
};
#endif