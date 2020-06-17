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
                    _modulesOnInterface[address - 0x70].push_back(new VM208EX(_interfaces[address - 0x70].getSocket(socket-3)));
                    auto interface = _modulesOnInterface[address - 0x70];
                    interface[interface.size() - 1]->initialize();
                }
            }
        }
    }
}

RelayModule* ModuleManager::getModule(int index)
{
    uint8_t checkedModules;
    uint8_t checkedInterfaces =0;
    uint8_t previousAmountChecked = 0;
    while(_modulesOnInterface[checkedInterfaces].size()+previousAmountChecked<index)
    {
        previousAmountChecked += _modulesOnInterface[checkedInterfaces].size();
        checkedInterfaces++;
    }
    return _modulesOnInterface[checkedInterfaces][index-previousAmountChecked];
}

ModuleManager::~ModuleManager()
{
    this->_modules.clear();
}

uint8_t ModuleManager::getAmount()
{
    uint8_t amount=0;
    for(int i=0;i<8;i++)
    {
        amount +=_modulesOnInterface[i].size();
    }
    return amount;
}

VM208* ModuleManager::getBaseModule()
{
    return _baseModule;
}

RelayModule* ModuleManager::getModuleFromInterface(uint8_t interface,uint8_t module)
{
    return _modulesOnInterface[interface][module];
}

uint8_t ModuleManager::getAmountOfModulesOnInterface(uint8_t interface)
{
    auto size = _modulesOnInterface[interface].size();
    Serial.printf("Amount of Modules on Interface %i is %i\r\n",interface,size);
    return size;
}