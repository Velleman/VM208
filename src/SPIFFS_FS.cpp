#include "SPIFFS_FS.hpp"


SPIFFS_Fs::SPIFFS_Fs()
{

}

void SPIFFS_Fs::begin()
{
    SPIFFS.begin();
}
File SPIFFS_Fs::open(String path,const char* mode)
{
    return SPIFFS.open(path,mode);
}
bool SPIFFS_Fs::exists(String path){
    return SPIFFS.exists(path);
}
void SPIFFS_Fs::remove(String path){
    SPIFFS.remove(path);
}