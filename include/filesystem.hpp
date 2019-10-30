#ifndef _FILESYSTEM_HPP_
#define _FILESYSTEM_HPP_
#include "FS.h"
class FileSystem{

public:
    virtual void begin();
    virtual File open(String path,const char* mode);
    virtual bool exists(String path);
    virtual void remove(String path);
};
#endif