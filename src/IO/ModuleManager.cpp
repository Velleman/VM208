#include "ModuleManager.hpp"
#include "VM208EX.h"
#include "VM208.h"
#include "global.hpp"
ModuleManager::ModuleManager()
{
}

void ModuleManager::DetectModules()
{

    if (_demo)
    {
        uint _channelIndex = 0;
        _baseModule = new VM208();
        _baseModule->initialize();
        _modules.push_back(_baseModule);
        for (int i = 0; i < 4; i++)
        {
            auto name = config.getNameFromChannel(_channelIndex);
            _baseModule->getChannel(i)->setName(name);
            Serial.println(config.getNameFromChannel(_channelIndex));
            _channelIndex++;
        }
        _channelIndex +=8; //skip VM208EX
        for (byte address = 0x70; address < 0x78; ++address)
        {
            _interfaceDetected++;
            _interfaces[address - 0x70].setAddress(address);
            for (byte socket = 4; socket < 8; socket++)
            {
                VM208EX *module = new VM208EX(_interfaces[address - 0x70].getSocket(socket - 3));
                _modulesOnInterface[address - 0x70].push_back(module);
                auto interface = _modulesOnInterface[address - 0x70];
                interface[interface.size() - 1]->initialize();
                for (int i = 0; i < 8; i++)
                {
                    auto name = config.getNameFromChannel(_channelIndex);
                    module->getChannel(i)->setName(name);
                    _channelIndex++;
                }
            }
        }
    }
    else
    {
        _baseModule = new VM208();
        _modules.push_back(_baseModule);
        _baseModule->initialize();
        _baseModule->turnAllChannelsOff();
        uint _channelIndex = 0;
        for (int i = 0; i < 4; i++)
        {
            auto name = config.getNameFromChannel(_channelIndex);
            _baseModule->getChannel(i)->setName(name);
            Serial.println(config.getNameFromChannel(_channelIndex));
            _channelIndex++;
        }
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
            _extensionModule = new VM208EX();
            _modules.push_back(_extensionModule);
            _extensionModule->initialize();
            _extensionModule->turnAllChannelsOff();
            for (int i = 0; i < 8; i++)
            {
                auto name = config.getNameFromChannel(_channelIndex);
                _extensionModule->getChannel(i)->setName(name);
                _channelIndex++;
            }
        }
        else
        {
            _channelIndex += 8;
        }
        for (byte address = 0x70; address < 0x72; ++address)
        {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();

            Serial.print("address: ");
            Serial.println(address, HEX);
            Serial.print("return value I2C ");
            Serial.println(error);
            if (!error)
            {
                _interfaceDetected++;
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
                        Serial.println(socket - 3);
                        VM208EX *module = new VM208EX(_interfaces[address - 0x70].getSocket(socket - 3));
                        _modulesOnInterface[address - 0x70].push_back(module);
                        auto interface = _modulesOnInterface[address - 0x70];
                        interface[interface.size() - 1]->initialize();
                        for (int i = 0; i < 8; i++)
                        {
                            auto name =config.getNameFromChannel(_channelIndex);
                            module->getChannel(i)->setName(name);
                            _channelIndex++;
                        }
                    }
                    else
                    {
                        _channelIndex += 8; //we skipped a module
                    }
                }
            }
            else
            {
                _channelIndex += 32; //we skipped 32 relays (4 modules with 8 channels)
            }
        }
    }
}

RelayModule *ModuleManager::getModule(int index)
{
    uint8_t checkedModules;
    uint8_t checkedInterfaces = 0;
    uint8_t previousAmountChecked = 0;
    /*if (index != 0)
    {
        if (index == 1 && _extensionModule != nullptr)
        {
            return _extensionModule;
        }
        while (_modulesOnInterface[checkedInterfaces].size() + previousAmountChecked < index)
        {
            previousAmountChecked += _modulesOnInterface[checkedInterfaces].size();
            checkedInterfaces++;
        }
        return _modulesOnInterface[checkedInterfaces][index - previousAmountChecked];
    }
    else
    {
        return _baseModule;
    }*/
    if(index == 0)
    {
        return _baseModule;
    }else if(index == 1)
    {
        return _extensionModule;
    }else{
        index -=2;
        auto interface = index/4;
        auto socket = index - (interface*4);
        return _modulesOnInterface[interface][socket];
    }
}

ModuleManager::~ModuleManager()
{
    this->_modules.clear();
}

uint8_t ModuleManager::getAmount()
{
    uint8_t amount = 0;
    amount += _modules.size();
    for (int i = 0; i < 8; i++)
    {
        amount += _modulesOnInterface[i].size();
    }
    return amount;
}

VM208 *ModuleManager::getBaseModule()
{
    return _baseModule;
}

RelayModule *ModuleManager::getModuleFromInterface(uint8_t interface, uint8_t module)
{
    /*if(8 > interface)
    {
        if(_modulesOnInterface[interface].size() > module)
        {
            return _modulesOnInterface[interface][module];
        }
    }
    return nullptr;  */
    return _modulesOnInterface[interface][module];
}

uint8_t ModuleManager::getAmountOfModulesOnInterface(uint8_t interface)
{
    auto size = _modulesOnInterface[interface].size();
    Serial.println(size);
    return size;
}

RelayChannel *ModuleManager::getChannel(int channelId)
{

    if (channelId >= 1 && channelId <= 4)
    {
        Serial.printf("Channel ID %d is baseModule Channel %d\r\n", channelId, channelId);
        return _baseModule->getChannel(channelId - 1);
    }
    else
    {
        if (_extensionModule != nullptr)
        {
            return _extensionModule->getChannel(channelId - 1);
        }
        else
        {
            if(channelId < 13)
                return nullptr;
            int index = channelId - 13;
            uint interface = floor(index / 32);
            uint module = floor((index - (interface * 32)) / 8);
            uint channel = index % 8;
            //Serial.printf("Channel ID %d is interface %d Module %d Channel %d\r\n", channelId, interface, module, channel);
            if(interface < _interfaceDetected)
            {
                if(module < _modulesOnInterface[interface].size())
                {
                    return ((VM208EX *)_modulesOnInterface[interface][module])->getChannel(channel);
                }else{
                }
                
            }else{
            }
            return nullptr;
        }
    }
}

uint16_t ModuleManager::getChannelId(uint8_t interface, uint8_t module, uint8_t channel)
{
    uint16_t channelId = 1;
    if (interface == 0)
    {
        if (module == 1)
        {
            channelId = channel + 4;
        }
        else
        {
            channelId += channel;
        }
    }
    else
    {
        uint16_t channels = (interface - 1) * 32;
        channels += module * 8;
        channels += channel;
        channels += 13;
        channelId = channels;
    }
    Serial.printf("Channel ID is %d", channelId);
    return channelId;
} 

bool ModuleManager::isExtensionConnected()
{
    return _extensionModule != nullptr;
}