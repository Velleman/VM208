#include "output.hpp"
#include "esp_timer.h"
#include "esp_err.h"
#include "IO.hpp"

extern "C"
{
static void timerCallback(void* arg)
{
    OutputId_t* oID = (OutputId_t*)arg;
    Relay* r = getRelayById(oID->id);
    r->toggle();
}
}
Output::Output(uint8_t id, uint16_t pin, bool initState, TCA6424A_TS *tca) : m_id(id),
                                                                          m_pin(pin),
                                                                          m_state(initState),
                                                                          m_tca(tca),
                                                                          m_isAccessible(false),
                                                                          m_pulseTime(5),
                                                                          m_timerTime(1)
{
    initPin();
    updateTCA();
    Serial.println(m_id);
}

uint8_t Output::getId()
{
    return m_id;
}

void Output::turnOff()
{
    m_state = false;
    updateTCA();
}

void Output::turnOn()
{
    m_state = true;
    updateTCA();
}

void Output::setState(bool state)
{
    m_state = state;
    updateTCA();
}

void Output::updateTCA()
{
    if (m_tca != nullptr && m_isAccessible)
    {
        m_tca->writePin(m_pin, m_state);
    }
}

void Output::toggle()
{
    m_state = !m_state;
    updateTCA();
}

void Output::initPin()
{
    if (m_tca != nullptr)
    {
        if (m_tca->testConnection())
        {
            m_isAccessible = true;
            m_tca->setPinDirection(m_pin, TCA6424A_OUTPUT);
        }
        else
        {
            m_isAccessible = false;
        }
    }
}

bool Output::getState()
{
    return m_state;
}

void Output::setPulseTime(uint16_t time)
{
    time *= 1000;
    m_pulseTime = (time <= 60000000) ? time : 60000000;
}

void Output::activatePulse()
{
    toggle();
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, m_pulseTime));
}

void Output::activatePulse(uint16_t time)
{
    Serial.print("PULSE");
    Serial.println(m_id);
    uint64_t t = time * 1000;
    uint16_t pulseTime = (t <= 60000000) ? t : 60000000;
    toggle();
    oID.id = m_id;
    const esp_timer_create_args_t oneshot_timer_args = {
            .callback = &timerCallback,
            .arg = &oID,
            .dispatch_method = ESP_TIMER_TASK,
            /* argument specified here will be passed to timer callback function */
            .name = "one-shot"
    };
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, pulseTime));
}

void Output::setTimerTime(uint16_t time)
{
    m_timerTime = (time <= 720) ? time : 720;
}

void Output::activateTimer()
{
    unsigned long time = m_timerTime*60000;
    toggle();
    delay(time);
    toggle();
}


void Output::activateTimer(uint16_t time)
{
    unsigned long time_ms = time*60000; //1 = 1min => 60000ms;
    toggle();
    delay(time_ms);
    toggle();
}

