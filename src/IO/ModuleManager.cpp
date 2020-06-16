#include "ModuleManager.hpp"
#include "VM208EX.h"
#include "VM208.h"
ModuleManager::ModuleManager()
{

}

void ModuleManager::DetectModules()
{
    _baseModule = new VM208();
    _modules.push_back(_baseModule);
    _baseModule->initialize();
    for (byte i = 0x70; i < 0x78; i++)
    {
        Wire.beginTransmission(i);
        Wire.write(0x00);
        Wire.endTransmission();
    }
    Wire.beginTransmission(0x23);
    byte error = Wire.endTransmission();
    Serial.print("return value I2C: ");
    Serial.println(error);
    if (!error)
    {
        Serial.println("native VM208EX found");
        _modules.push_back(new VM208EX());
        _modules[_modules.size() - 1]->initialize();
    }
    for (byte address = 0x70; address < 0x78; ++address)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        Serial.print("address: ");
        Serial.println(address, HEX);
        Serial.print("return value I2C ");
        Serial.println(error);
        if (!error)
        {
            _interfaces[address - 0x70].setAddress(address);
            for (byte socket = 4; socket < 8; socket++)
            {
                Wire.beginTransmission(address);
                Wire.write(socket);
                Wire.endTransmission();

                Wire.beginTransmission(0x23);
                error = Wire.endTransmission();
                if (!error)
                {
                    Serial.print("VM208EX found on interface address: ");
                    Serial.println(address, HEX);
                    Serial.print("socket: ");
                    Serial.println(socket-3);
                    _modules.push_back(new VM208EX(_interfaces[address - 0x70].getSocket(socket-3)));
                    _modules[_modules.size() - 1]->initialize();
                }
            }
        }
    }
}

RelayModule* ModuleManager::getModule(int index)
{
    return _modules[index];
}

ModuleManager::~ModuleManager()
{
    this->_modules.clear();
}

uint8_t ModuleManager::getAmount()
{
    return this->_modules.size();
}

VM208* ModuleManager::getBaseModule()
{
    return _baseModule;
}