#include "mail.hpp"

#include "global.hpp"
#include "ETH.h"

//The Email Sending data object contains config and data to send
SMTPData smtpData;

//Callback function to get the Email sending status
void sendCallback(SendStatus info);

void sendEmail(void *pvParamaters)
{
  xSemaphoreTake(g_MutexMail, portMAX_DELAY);
  Serial.println(config.getEmailServer());
  Serial.println(config.getEmailPort());
  Serial.println(config.getEmailUser());
  Serial.println(config.getEmailPW());
  Serial.println(config.getEmailRecipient());
  //Set the Email host, port, account and password
  smtpData.setLogin(config.getEmailServer(), config.getEmailPort().toInt(), config.getEmailUser(), config.getEmailPW());

  //Set the sender name and Email
  smtpData.setSender("ESP32", config.getEmailUser());

  //Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  //Set the subject
  smtpData.setSubject("ESP32 SMTP Mail Sending Test");

  //Set the message - normal text or html format
  uint8_t notification = (uint32_t)pvParamaters;
  String message;
  Serial.println(notification);
  switch (notification)
  {
  case MAN_INPUT_MAIL:
    message = "<div style=\"color:#ff0000;font-size:20px;\">A button has been pressed in the last minute - From VM208</div>";
    break;
  case BOOT_MAIL:
    message = "<div style=\"color:#ff0000;font-size:20px;\">Booted Up! - From VM208</div>";
    break;
  case INPUT_MAIL:
    message = "<div style=\"color:#ff0000;font-size:20px;\">Input Changed! - From VM208</div>";
    break;
  case EXT_MAIL:
    message = "<div style=\"color:#ff0000;font-size:20px;\">Extension Connected! - From VM208</div>";
    break;
  case EXT_DIS_MAIL:
    message = "<div style=\"color:#ff0000;font-size:20px;\">Extension Disconnected! - From VM208</div>";
    break;
  default:
    message = "<div style=\"color:#ff0000;font-size:20px;\">You should not be receiving this! Oopsie :( - From VM208</div>";
    break;
  }
  smtpData.setMessage(message, true);

  //Add recipients, can add more than one recipient
  smtpData.addRecipient(config.getEmailRecipient());

  //smtpData.setSendCallback(sendCallback);

  Serial.println("Sending Mail...");
  //Start sending Email, can be set callback function to track the status
  smtpData.setSendCallback(sendCallback);

  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  //Clear all data from Email object to free memory
  smtpData.empty();
  xSemaphoreGive(g_MutexMail);
  vTaskDelete(NULL);
}

//Callback function to get the Email sending status
void sendCallback(SendStatus msg)
{
  //Print the current status
  Serial.println(msg.info());

  //Do something when complete
  if (msg.success())
  {
    Serial.println("----------------");
  }
}

void sendManualInputMail()
{
  static unsigned long previousTime = 0;
  if (config.getNotification_manual_input())
  {
    //check if a minute has passed
    if((millis() - previousTime) > 60000)
    {
      xTaskCreate(sendEmail, "send_mail", 6144, (void *)MAN_INPUT_MAIL, (tskIDLE_PRIORITY +  10), NULL);
      previousTime = millis();
    }
  }
}

void sendBootMail()
{
  if (config.getNotificationBoot())
    xTaskCreate(sendEmail, "send_mail", 6144, (void *)BOOT_MAIL, (tskIDLE_PRIORITY + 10), NULL);
}

void sendInputChangedMail()
{
  if (config.getNotificationInputChange())
    xTaskCreate(sendEmail, "send_mail", 6144, (void *)INPUT_MAIL, (tskIDLE_PRIORITY + 10), NULL);
}

void sendExtConnectedMail()
{
  if (config.getNotification_ext_connected())
  {
    Serial.println("EXT MAIL WILL BE SEND");
    xTaskCreate(sendEmail, "send_mail", 6144, (void *)EXT_MAIL, (tskIDLE_PRIORITY + 10), NULL);
  }
  else
  {
    Serial.println("EXT MAIL WILL BE NOT SEND");
  }
}

void sendExtDisConnectedMail()
{
  if (config.getNotification_ext_connected())
  {
    Serial.println("EXT MAIL WILL BE SEND");
    xTaskCreate(sendEmail, "send_mail", 6144, (void *)EXT_DIS_MAIL, (tskIDLE_PRIORITY + 10), NULL);
  }
  else
  {
    Serial.println("EXT MAIL WILL BE NOT SEND");
  }
}