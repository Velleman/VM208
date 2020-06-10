#include "ModuleManager.hpp"

ModuleManager::ModuleManager()
{
}

void ModuleManager::DetectModules()
{
    Wire.beginTransmission(0x23);
    byte error = Wire.endTransmission();
    if (!error)
    {
        _modules.push_back(VM208EX());
    }
    for (byte address = 70; address < 78; ++address)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (!error)
        {
            _interfaces[address-70].setAddress(address);
            for (byte socket = 1; socket < 5; socket++)
            {
                Wire.beginTransmission(address);
                Wire.write(socket);
                Wire.endTransmission();

                Wire.beginTransmission(0x23);
                error = Wire.endTransmission();
                if(!error)
                {
                    _modules.push_back(VM208EX(_interfaces[address-70].getSocket(socket)));
                }
            }
        }
    }
}

ModuleManager::~ModuleManager()
{
    this->_modules.clear();
}