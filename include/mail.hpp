#ifndef _MAIL_HPP
#define _MAIL_HPP
#include "ESP32_MailClient.h"
#define MAN_INPUT_MAIL 1
#define BOOT_MAIL 2
#define INPUT_MAIL 3
#define EXT_MAIL 4
#define EXT_DIS_MAIL 5
#define TEST_MAIL 6
void sendEmail(void *pvParamaters);
void sendCallback(SendStatus msg);
void sendManualInputMail();
void sendBootMail();
void sendInputChangedMail();
void sendExtConnectedMail();
void sendExtDisConnectedMail();
void sendTestMail();
#endif