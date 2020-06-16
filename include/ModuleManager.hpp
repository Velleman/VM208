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
        uint8_t getAmount();
        ~ModuleManager();
    private:
        VM208* _baseModule;
        vector<RelayModule *> _modules;
        VM208INT _interfaces[8];
};
