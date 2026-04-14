#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "main.h"

// Cấu trúc gom nhóm dữ liệu cảm biến
typedef struct {
    float temperature;
    uint32_t smoke_raw;
    float smoke_conc;
    uint8_t mh_sensor_do;      // Tín hiệu số DO của hồng ngoại (0 = Có lửa/vật cản, 1 = Bình thường)
    float mh_sensor_ao_volt;   // Tín hiệu điện áp AO (Volt)
} SensorData_t;

// Hàm nguyên mẫu cho service layer
void SensorService_Init(void);
void SensorService_Update(SensorData_t *data);

#endif /* SENSOR_SERVICE_H */
