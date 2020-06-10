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
        ~ModuleManager();
    private:
        vector<Module> _modules;
        VM208INT _interfaces[8];
};
