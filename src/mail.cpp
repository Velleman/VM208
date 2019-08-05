#include "mail.hpp"

#include "global.hpp"

//WiFi or HTTP client for internet connection
HTTPClientESP32Ex http;

//The Email Sending data object contains config and data to send
SMTPData smtpData;

//Callback function to get the Email sending status
void sendCallback(SendStatus info);

void sendEmail(void *pvParamaters)
{
  xSemaphoreTake(g_MutexMail,portMAX_DELAY);
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
  smtpData.setMessage("<div style=\"color:#ff0000;font-size:20px;\">Hello World! - From ESP32</div>", true);

  //Add recipients, can add more than one recipient
  smtpData.addRecipient(config.getEmailRecipient());

  smtpData.setSendCallback(sendCallback);

  Serial.println("Sending Mail...");
  //Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(http, smtpData))
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
  
}

void sendBootMail()
{

}

void sendInputChangedMail()
{

}

void sendExtConnectedMail()
{

}