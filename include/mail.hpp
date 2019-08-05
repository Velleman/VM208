#ifndef _MAIL_HPP
#define _MAIL_HPP
#include "ESP32_MailClient.h"
void sendEmail(void *pvParamaters);
void sendCallback(SendStatus msg);
void sendManualInputMail();
void sendBootMail();
void sendInputChangedMail();
void sendExtConnectedMail();
#endif