#ifndef OUTPUT_H
#define OUTPUT_H
#include "stdint.h"
#include "tca_thread_safe.hpp"
typedef struct
{
  uint8_t id;
} OutputId_t;

class Output
{
protected:
  OutputId_t oID;
  uint8_t m_id;
  uint16_t m_pin;
  bool m_state;
  void updateTCA();
  TCA6424A_TS *m_tca;
  bool m_isAccessible;
  uint64_t m_pulseTime;
  uint64_t m_timerTime;
  bool m_setState;
  
  void TaskTimer(void *pvParameters);
  void startSheduler();
public:
  Output(uint8_t id = 0, uint16_t pin = 0, bool initState = 0, TCA6424A_TS *tca = nullptr,bool setState = true);

  uint8_t getId();

  void turnOn(void);

  void turnOff(void);

  void toggle(void);

  void setState(bool state);

  void setPulseTime(uint16_t time);

  void activatePulse();

  void activatePulse(uint64_t time);

  void clearPulse();

  void setTimerTime(uint16_t time);

  void activateTimer();

  void activateTimer(uint64_t time);

  void clearTimer();
  
  void setSchedule(tm start_date, tm end_date);

  bool getState();

  void initPin(bool CheckConnection);
};
#endif