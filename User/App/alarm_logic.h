#ifndef ALARM_LOGIC_H
#define ALARM_LOGIC_H

#include "sensor_service.h"

// Ngưỡng báo động (Thresholds)
#define ALARM_TEMP_THRESHOLD 60.0f  // 60 độ C
#define ALARM_SMOKE_THRESHOLD 2.0f  // Mức điện áp tương ứng nồng độ nguy hiểm

typedef enum {
    ALARM_NONE = 0,
    ALARM_WARNING,
    ALARM_FIRE_CRITICAL
} AlarmLevel_t;

void AlarmLogic_Init(void);
AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data);
void Alarm_ExecuteAction(AlarmLevel_t level);

#endif /* ALARM_LOGIC_H */
