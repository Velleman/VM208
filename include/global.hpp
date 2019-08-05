#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include "FreeRTOS.h"
#include "config_vm208.hpp"
extern Configuration config;
extern SemaphoreHandle_t g_Mutex;
extern SemaphoreHandle_t g_MutexChannel;
extern SemaphoreHandle_t g_MutexMail;
extern bool gotETH_IP;
extern bool gotSTA_IP;
#endif