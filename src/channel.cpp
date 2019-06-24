#include "channel.hpp"
#include "IO.hpp"
#include "esp_err.h"
#include "esp_timer.h"
#include "global.hpp"
#include "config_vm208.hpp"

extern "C"
{

    static void timerCallback(void *arg)
    {
        OutputId_t *oID = (OutputId_t *)arg;
        Channel *c = getChannelById(oID->id);
        if (c != nullptr)
        {
            c->toggle();
        }
    }

    static void checkSheduler(void *pvParameter)
    {
        delay(500); // wait to load all channels
        uint8_t *id = (uint8_t *)pvParameter;
        Serial.print("Shedule channel id");
        Serial.println(*id);
        Channel *c = getChannelById(*id);
        while (true)
        {
            time_t t = time(NULL);
            struct tm *time = localtime(&t);
            for (int i = 0; i < 14; i++)
            {
                Alarm* a = c->getAlarm(i);
                if (a->isEnabled())
                {
                    if (time->tm_hour == a->getHour() && time->tm_min == a->getMinute && time->tm_wday == a->getWeekday())
                    {
                        a->getState() ? c->turnOn() : c->turnOff();
                    }
                }
            }
            delay(500);
        }
        vTaskDelete(NULL);
    }
}

Channel::Channel(String name, Relay *relay, Led *led, uint8_t id) : m_name(name), m_relay(relay), m_led(led), m_id(id)
{
}

void Channel::loadShedule()
{
    //config.
}

void Channel::startSheduler()
{
    xTaskCreate(checkSheduler, "Sheduler", 2048, &m_id, (tskIDLE_PRIORITY + 2), NULL);
}

void Channel::turnOn()
{
    m_relay->turnOn();
    if (m_led != nullptr)
        m_led->turnOn();
}

void Channel::turnOff()
{
    m_relay->turnOff();
    if (m_led != nullptr)
        m_led->turnOff();
}

void Channel::toggle()
{
    m_relay->toggle();
    if (m_led != nullptr)
        m_led->toggle();
}

void Channel::setName(String name)
{
    m_name = name;
}

void Channel::activateTimer(uint16_t timertime)
{
    toggle();
    unsigned long time_us = timertime * 60000000UL; //1 = 1min => 60000ms;
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &timerCallback,
        .arg = &m_id,
        .dispatch_method = ESP_TIMER_TASK,
        /* argument specified here will be passed to timer callback function */
        .name = "one-shot"};
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer_timer));
    Serial.print("Pulse Time:");
    Serial.println(time_us);
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer_timer, time_us));
}

void Channel::activatePulse(uint64_t pulsetime)
{

    uint32_t t = ((uint32_t)pulsetime) * ((uint32_t)1000);
    uint32_t pulseTime = (t <= 60000000) ? t : 60000000;
    toggle();
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &timerCallback,
        .arg = &m_id,
        .dispatch_method = ESP_TIMER_TASK,
        /* argument specified here will be passed to timer callback function */
        .name = "one-shot"};
    Serial.print("Pulse Time:");
    Serial.println(pulseTime);
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_pulse_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_pulse_timer, pulseTime));
}

void Channel::setAlarm(Alarm a, uint8_t index)
{
    if (index < 14)
    {
        alarms[index] = a;
    }
}

Alarm* Channel::getAlarm(uint8_t index)
{
    return alarms + index;
}