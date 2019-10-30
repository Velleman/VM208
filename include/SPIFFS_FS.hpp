#ifndef SPIFFS_FS_HPP
#define SPIFFS_FS_HPP
#include "filesystem.hpp"
#include "SPIFFS.h"
class SPIFFS_Fs : public FileSystem
{
private:
public:
    SPIFFS_Fs();
    void begin();
    File open(String path,const char* mode);
    bool exists(String path);
    void remove(String path);
};
#endif