#ifndef DS18B20_H
#define DS18B20_H

#include "main.h"

// Định nghĩa chân kết nối (Khớp với label bạn đã đặt trong CubeMX: GPIO_DATA_DB12)
#define DS18B20_PORT GPIO_DATA_DB12_GPIO_Port
#define DS18B20_PIN  GPIO_DATA_DB12_Pin

// Các hàm nguyên mẫu
void DS18B20_Init(void);
float DS18B20_ReadTemperature(void);
float DS18B20_ReadTemperature_NonBlocking(void);
uint8_t DS18B20_Start(void);
void DS18B20_Write(uint8_t data);
uint8_t DS18B20_Read(void);
void delay_us(uint32_t us);

#endif /* DS18B20_H */
