#ifndef MQ2_H
#define MQ2_H

#include "main.h"

// Các hàm nguyên mẫu cho MQ2
void MQ2_Init(void);
float MQ2_GetBaseVoltage(void);
uint32_t MQ2_ReadRawData(void);
float MQ2_CalculateGasConcentration(uint32_t raw_adc);

#endif /* MQ2_H */
