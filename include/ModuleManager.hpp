#pragma once
#include "VM208EX.h"
#include <list>
using namespace std;
class ModuleManager{
    public:
        ModuleManager();
        ~ModuleManager();
    private:
        list<VM208EX> _modules;

};
