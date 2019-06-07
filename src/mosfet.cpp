#include "mosfet.hpp"
#include "esp_log.h"

static const char* TAG = "MOSFET";

void Mosfet::turnOn()
{
    ESP_LOGI(TAG,"turn on");
    m_state = true;
    updateTCA();
}

void Mosfet::turnOff(){
    ESP_LOGI(TAG,"turn off");
    m_state = false;
    updateTCA();
}

void Mosfet::setState(bool state)
{
    ESP_LOGI(TAG,"change state");
    m_state = state;
    updateTCA();
}