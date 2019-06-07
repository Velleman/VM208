#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include "FreeRTOS.h"
#include "configuration.hpp"
extern Configuration config;
extern SemaphoreHandle_t g_Mutex;
extern bool gotETH_IP;
extern bool gotSTA_IP;
#endif