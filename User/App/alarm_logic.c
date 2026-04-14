#include "alarm_logic.h"
#include "main.h"

static uint8_t is_muted = 0; // Biến ghi nhớ trạng thái nhấn nút

void AlarmLogic_Init(void) {
    // Khởi tạo các còi, đèn LED báo cháy (nếu có)
}

AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data) {
    // Ngưỡng báo động (Có thể tùy chỉnh lại tại đây)
    uint8_t has_fire = (data->temperature > 50.0f && data->smoke_conc > 1.5f);
    uint8_t has_warning = (data->temperature > 40.0f || data->smoke_conc > 1.0f);

    if (has_fire || has_warning) {
        if (is_muted) return ALARM_NONE; // Nếu đã ấn nút thì coi như bình thường
        return has_fire ? ALARM_FIRE_CRITICAL : ALARM_WARNING;
    } else {
        is_muted = 0; // Khi hết nguy hiểm, tự động reset để lần sau báo động tiếp
        return ALARM_NONE;
    }
}

void Alarm_ExecuteAction(AlarmLevel_t level) {
    // 1. Kiểm tra nút nhấn (PC13 - Active High trên H5)
    // Tạm thời hard-code chân PC13 (Tránh lỗi chưa define label)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
        is_muted = 1;
    }

    // 2. Điều khiển LED Đỏ (PG4)
    // Hard-code chân PG4 để đảm bảo LED đỏ hoạt động
    if (level != ALARM_NONE) {
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
    }
}
