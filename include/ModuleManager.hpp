#pragma once
#include "VM208.h"
#include "VM208EX.h"
#include "VM208INT.h"
#include <vector>
using namespace std;
class ModuleManager{
    public:
        ModuleManager();
        void DetectModules();
    	VM208* getBaseModule();
        RelayModule* getModule(int index);
        RelayChannel* getChannel(int index);
        uint8_t getAmount();
        ~ModuleManager();
        RelayModule* getModuleFromInterface(uint8_t interface,uint8_t module);
        uint8_t getAmountOfModulesOnInterface(uint8_t interface);
        uint16_t getChannelId(uint8_t interface,uint8_t module,uint8_t channel);
        bool isExtensionConnected();
    private:
        VM208* _baseModule;
        VM208EX* _extensionModule = nullptr;
        vector<RelayModule *> _modules;
        vector<RelayModule *> _modulesOnInterface[8];
        VM208INT _interfaces[8];
        const bool _demo = true;
        uint8_t _interfaceDetected = 0;
};
