#pragma once
#include "VM208.h"
#include "alarm.hpp"
class VM208TimeChannel : public VM208Channel
{
public:
    VM208TimeChannel();
    void activateTimer(uint16_t time);
    void activatePulse(uint64_t time);
    void setAlarm(Alarm alarm, uint8_t index);
    Alarm *getAlarm(uint8_t index);
    void clearTimerAndPulse();
    bool isTimerActive();
    bool isPulseActive();
    uint64_t getTimerTime();
    uint64_t getPulseTime();
    void clearTimer();
    void clearPulse();
    void disableSheduler();
    bool isSheduleActive();

private:
    esp_timer_handle_t oneshot_pulse_timer;
    esp_timer_handle_t oneshot_timer_timer;
    Alarm alarms[14];
    bool m_isTimerActive;
    bool m_isPulseActive;
    uint64_t m_pulseTime;
    uint64_t m_timerTime;
    TaskHandle_t pulseTaskHandle;
    TaskHandle_t timerTaskHandle;
};