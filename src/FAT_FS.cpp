#include "FAT_FS.hpp"


FAT_Fs::FAT_Fs() : FileSystem()
{

}

void FAT_Fs::begin()
{
    if(!FFat.begin()){
        Serial.println("FFat Mount Failed");
        return;
    }
}
File FAT_Fs::open(String path,const char* mode)
{
    return FFat.open(path, FILE_WRITE);
}
bool FAT_Fs::exist(String path){
    return FFat.exists(path);
}
void FAT_Fs::remove(String path){
    FFat.remove(path);
}