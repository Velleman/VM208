/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.hpp"
#include <I2CDev.h>
#include "tca_thread_safe.hpp"
#include "esp_log.h"
#include "global.hpp"
#include "mail.hpp"
#include "ETH.h"
#include "VM208.h"
#include "VM208EX.h"
VM208 vm208;
VM208EX vm208ex;
xQueueHandle int_evt_queue = NULL;

static void gpio_isr_handler(void *arg)
{
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(int_evt_queue, &gpio_num, NULL);
}

void Init_IO(bool setState)
{

  Wire.begin(33, 32, 100000);
  vm208 = *(new VM208());
  vm208ex = *(new VM208EX());

  /*gpio_pullup_en(GPIO_NUM_33);
  gpio_pullup_en(GPIO_NUM_32);
  gpio_pulldown_dis(GPIO_NUM_33);
  gpio_pulldown_dis(GPIO_NUM_32);*/

  pinMode(4, INPUT);

  //Init Interrupt Pin
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  //bit mask of the pins, use GPIO4/35 here
  io_conf.pin_bit_mask = (1ULL << 4);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&io_conf);

  gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_35);
  gpio_set_intr_type(GPIO_NUM_35, GPIO_INTR_NEGEDGE);

  //create a queue to handle gpio event from isr
  int_evt_queue = xQueueCreate(2, sizeof(uint32_t));
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(INT_PIN, gpio_isr_handler, (void *)INT_PIN);
  /*if (tca_ext.ts_testConnection())
  {
    ESP_LOGI(TAG, "EXTENSION CONNECTED");
    gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void *)INT2_PIN);
  }*/
  Serial.println("Blink start");
  for (int i = 0; i < 4; i++)
  {
    
  }
  vm208.turnAllChannelsOn();
  vm208ex.turnAllChannelsOn();
  delay(1000);
  vm208.turnAllChannelsOff();
  vm208ex.turnAllChannelsOff();
  delay(1000);
  vm208.turnAllChannelsOn();
  vm208ex.turnAllChannelsOn();
  delay(1000);
  vm208.turnAllChannelsOff();
  vm208ex.turnAllChannelsOff();
  Serial.println("Blink end");
}

void initExtPinDirections()
{
  /*ESP_LOGI(TAG, "initExtPinDirections");
  for (int i = 4; i < 12; i++)
  {
    currentInputs[i]->initPin(false);
    relays[i].initPin(false);
    leds[i].initPin(false);
  }*/
}

Input **readInputs(Input **inputs)
{
  for (int i = 0; i < INPUT_MAX; i++)
  {
    inputs[i]->read();
  }
  return inputs;
}

void IO_task(void *arg)
{

  uint32_t io_num;
  //xTaskCreate(updateIO, "checkExtension", 4096, NULL, (tskIDLE_PRIORITY + 2), NULL);
  unsigned long previousTime = 0;
  unsigned long previousTime2 = 0;
  while (1)
  {

    if (xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY)||digitalRead(INT_PIN) == LOW)
    {

      if (io_num == INT2_PIN) //read extension
      {
        
      }
      else
      {
          toggleChannel(vm208, vm208.getPressedButton());
      }
    }
    io_num = 0;
  }
  delay(100);
}

void toggleChannel(VM208 &ex, uint8_t channel)
{
  if (channel)
  {
    ex[channel - 1].toggle();
  }
}

void updateIO(void *params)
{
  for (;;)
  {
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    /*for(int i =0;i<12;i++)
    {
      channels[i].toggle();
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
    for(int i =0;i<12;i++)
    {
      channels[i].toggle();
    }
    vTaskDelay(500/portTICK_PERIOD_MS);*/
  }
  vTaskDelete(NULL);
}

Relay *getRelays()
{
  return nullptr;
}

void getLeds(Led *pLeds)
{
  pLeds = nullptr;
}

bool IsExtensionConnected()
{
  /*bool currentState = tca_ext.ts_testConnection();
  if (currentState == true && previousExtConnectedState == false)
  {
    delay(1000);
    initExtPinDirections();
    copyStateRelaysToLeds();
    uint8_t data[3];
    tca_ext.readAll(data);
    gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void *)INT2_PIN);
    Serial.println("EXT CONNECTED");
    if (WiFi.isConnected() || ETH.linkUp())
      sendExtConnectedMail();
  }
  if (previousExtConnectedState == true && currentState == false)
  {
    Serial.println("EXT NOT CONNECTED");
    gpio_isr_handler_remove(INT2_PIN);
    gpio_pullup_en(INT2_PIN);
    if (WiFi.isConnected() || ETH.linkUp())
      sendExtDisConnectedMail();
  }
  previousExtConnectedState = currentState;
  return currentState;*/
  return false;
}

Mosfet *getMosfetById(int id)
{
   return nullptr;
}

Relay *getRelayById(int id)
{
  return nullptr;
}

Channel *getChannelById(int id)
{
  /*xSemaphoreTake(g_MutexChannel, portMAX_DELAY);
  if (id <= 12 && id > 0)
  {
    xSemaphoreGive(g_MutexChannel);
    return &channels[id - 1];
  }
  else
  {
    ESP_LOGE("IO", "Channel ID is invalid: id is %i", id);
    xSemaphoreGive(g_MutexChannel);
    return nullptr;
  }*/
  return nullptr;
}

VM208* getVM208()
{
  return &vm208;
}