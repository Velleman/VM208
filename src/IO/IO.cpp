/* IO.c

@Author: BN
Copyright 2019 Velleman nv
*/
#include "IO.hpp"
#include <I2CDev.h>
//#include "tca_thread_safe.hpp"
#include "esp_log.h"
#include "mail.hpp"
#include "ETH.h"
#include "PulseAndTimer.hpp"
#include "global.hpp"

xQueueHandle int_evt_queue = NULL;
unsigned long previousTime = 0;
QueueHandle_t timerQueue;
QueueHandle_t timerStopQueue;
QueueHandle_t timerStatusQueue;
QueueHandle_t pulseQueue;
QueueHandle_t pulseStopQueue;
QueueHandle_t pulseStatusQueue;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
  if ((millis() - previousTime) > 50)
  {
    previousTime = millis();
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(int_evt_queue, &gpio_num, NULL);
  }
}

void Init_IO(bool setState)
{

  Wire.begin(33, 32, 10000);
  Wire.setTimeOut(100);
  xSemaphoreTake(g_Mutex,1000/portTICK_PERIOD_MS);
  mm.DetectModules();
  xSemaphoreGive(g_Mutex);
  config.loadAlarms();
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

  /*gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);
  gpio_pullup_en(GPIO_NUM_35);
  pinMode(35,INPUT);
  gpio_set_intr_type(GPIO_NUM_35, GPIO_INTR_NEGEDGE);*/
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  //bit mask of the pins, use GPIO4/35 here
  io_conf.pin_bit_mask = (1ULL << 35);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&io_conf);

  //create a queue to handle gpio event from isr
  int_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(INT_PIN, gpio_isr_handler, (void *)INT_PIN);

  if (mm.isExtensionConnected()) //if a module is detected the second interrupt pin is used.
  {
    gpio_isr_handler_add(INT2_PIN, gpio_isr_handler, (void *)INT2_PIN);
  }

  Serial.println("Found modules: ");
  Serial.println(mm.getAmount());
  timerQueue = xQueueCreate(10, sizeof(TimeParameters_t));
  timerStopQueue = xQueueCreate(10, sizeof(long));
  pulseStopQueue = xQueueCreate(10, sizeof(long));
  pulseQueue = xQueueCreate(10, sizeof(TimeParameters_t));
  pulseStatusQueue = xQueueCreate(1, sizeof(pulseStatus_t));
  timerStatusQueue = xQueueCreate(1, sizeof(timerStatus_t));
  xTaskCreate(timerTask, "timer", 6144, NULL, (tskIDLE_PRIORITY + 3), NULL);
  xTaskCreate(pulseTask, "pulse", 6144, NULL, (tskIDLE_PRIORITY + 5), NULL);
}

/*Input **readInputs(Input **inputs)
{
  for (int i = 0; i < INPUT_MAX; i++)
  {
    inputs[i]->read();
  }
  return inputs;
}*/

void IO_task(void *arg)
{

  uint32_t io_num;
  while (1)
  {
    if (xQueueReceive(int_evt_queue, &io_num, portMAX_DELAY / portTICK_PERIOD_MS)) //|| digitalRead(INT_PIN) == LOW || digitalRead(INT2_PIN) == LOW)
    {
      
      if (io_num == INT2_PIN )//|| digitalRead(INT2_PIN) == LOW) //read extension
      {
        Serial.print("Pin is low:");
        if (mm.isExtensionConnected())
        {
          Serial.println("VM208EX");
          toggleChannel((VM208EX *)mm.getModule(1), mm.getModule(1)->getPressedButton());
        }
        else
        {
          Serial.println("Extentions");
          /*for (int i = 0; i < 8; i++) // go over each interface
          {
            xSemaphoreTake(g_Mutex,1000/portTICK_PERIOD_MS);
            uint8_t socket = mm.getInterface(i)->handleInterrupt();
            
            for (int j = 0; j < 4; j++)
            {
              auto interrruptTriggered = (socket >> j) & 0x01;
              if (interrruptTriggered) //if 0 then no interrupt happend on that interface, socket == module ID
              {
                mm.getModuleFromInterface(i, j)->Activate();
                auto button = mm.getModuleFromInterface(i, j)->getPressedButton(); //From Interface I take module and retreive the pressed button
                if (button != 0)
                {
                  (*((VM208EX *)mm.getModuleFromInterface(i, j)))[button - 1].toggle(); //use the button id to toggle it
                }
                mm.getModuleFromInterface(i, j)->Disactivate();
              }
            }
            xSemaphoreGive(g_Mutex);
          }*/
        }
      }
      else
      {
        Serial.println("Base");
        if (xSemaphoreTake(g_Mutex, 1000 / portTICK_PERIOD_MS))
        {
          uint8_t channel = mm.getBaseModule()->getPressedButton();
          xSemaphoreGive(g_Mutex);
          toggleChannel(mm.getBaseModule(), channel);          
        }
      }
    }
    io_num = 0;
    delay(50);
  }
}

void toggleChannel(VM208 *ex, uint8_t channel)
{
  if (xSemaphoreTake(g_Mutex, 1000 / portTICK_PERIOD_MS))
  {
    if (channel && channel < 5)
    {
      VM208 vm = *ex;
      vm[channel - 1].toggle();
    }
    xSemaphoreGive(g_Mutex);
  }
}

void toggleChannel(VM208EX *ex, uint8_t channel)
{
  if (xSemaphoreTake(g_Mutex, 1000 / portTICK_PERIOD_MS))
  {
    if (channel)
    {
      (*ex)[channel - 1].toggle();
      //(*(ex))[channel - 1].toggle();
    }
    xSemaphoreGive(g_Mutex);
  }
}

RelayChannel *getRelayChannelById(int id)
{
  if (xSemaphoreTake(g_Mutex, 100 / portTICK_PERIOD_MS))
  {
    if (id <= 4)
    {
      VM208 *vm208 = (VM208 *)mm.getModule(0);
      return &(*vm208)[id - 1];
    }
    else
    {
      VM208EX *ex = (VM208EX *)mm.getModule(id / 8);
      return &(*ex)[id % 8];
    }
    xSemaphoreGive(g_Mutex);
  }
  return nullptr;
}