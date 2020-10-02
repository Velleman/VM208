#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include "config_vm208.hpp"
#include <DNSServer.h>
#include "ModuleManager.hpp"
extern EventGroupHandle_t s_wifi_event_group;
extern Configuration config;
extern SemaphoreHandle_t g_Mutex;
extern SemaphoreHandle_t g_MutexChannel;
extern SemaphoreHandle_t g_MutexMail;
extern QueueHandle_t timerQueue;
extern QueueHandle_t timerStopQueue;
extern QueueHandle_t pulseQueue;
extern QueueHandle_t pulseStopQueue;
extern QueueHandle_t pulseStatusQueue;
extern QueueHandle_t timerStatusQueue;
extern bool gotETH_IP;
extern bool gotSTA_IP;
extern DNSServer dnsServer;
extern ModuleManager mm;
struct timerParam_t
{
    bool enabled;
    unsigned long endTime;
};
#endif