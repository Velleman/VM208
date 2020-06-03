#include "time_VM208.hpp"
#include "Arduino.h"


void printLocalTime()
{
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void time_keeping_task(void *pvParameter)
{
  while (true)
  {
    time_t now;
    struct tm *timeInfo;
    now = time(nullptr);
    timeInfo = localtime(&now);
    if (timeInfo->tm_hour == 3 && timeInfo->tm_min == 5)
    {
      Serial.println("Daily Time Check");
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo))
      {
        Serial.println("Failed to obtain time");
        return;
      }
    }
    delay(500);
  }
  vTaskDelete(NULL);
}