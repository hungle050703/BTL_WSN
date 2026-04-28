#ifndef __A7677_H__
#define __A7677_H__

#include "main.h"

// Tên hàm chung cho dòng A7677 / A7677
void A7677_PowerOn(void);
void A7677_Send_AT(const char* cmd);
void A7677_Call(const char* phone_number);
void A7677_SendSMS(const char* phone_number, const char* message);

#endif /* __A7677_H__ */