#include "mq2.h"

extern ADC_HandleTypeDef hadc1;

// Khai báo biến lưu mức nền chuẩn
static float mq2_base_voltage = 0.0f;
static float mq2_filtered_voltage = 0.0f;

void MQ2_Init(void) {
    // Khởi tạo MQ2 nếu cần thiết (ví dụ bật heater, hiệu chuẩn ban đầu)
    // Tự động Calibration bằng code ở đây nếu có
    
    // Đọc trung bình 10 lần lấy mức nền khởi động (Calibration)
    float sum_volt = 0.0f;
    for(int i=0; i<10; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            uint32_t val = HAL_ADC_GetValue(&hadc1);
            sum_volt += (float)val * (3.3f / 4095.0f) * 1.47f; // Tính luôn Volt thật
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(100); // Đợi 100ms mỗi lần đọc
    }
    mq2_base_voltage = sum_volt / 10.0f;
    mq2_filtered_voltage = mq2_base_voltage; // Khởi tạo bộ lọc
}

float MQ2_GetBaseVoltage(void) {
    return mq2_base_voltage;
}

uint32_t MQ2_ReadRawData(void) {
    // Dừng ADC trước để xóa mọi cờ lỗi (đặc biệt là cờ Overrun - OVR)
    HAL_ADC_Stop(&hadc1);
    
    // Bắt đầu quá trình lấy mẫu mới
    HAL_ADC_Start(&hadc1);
    
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) { // Chờ tối đa 10ms
        uint32_t val = HAL_ADC_GetValue(&hadc1); 
        
        // Đọc xong phải DỪNG NGAY LẬP TỨC để ngăn ADC chạy tiếp gây lỗi Overrun
        HAL_ADC_Stop(&hadc1);
        
        return val;
    }
    
    return 0; // Lỗi đọc ADC
}

float MQ2_CalculateGasConcentration(uint32_t raw_adc) {
    // Điện áp đọc được tại chân vi điều khiển (đã bị chia qua cầu phân áp)
    float v_at_pin = (float)raw_adc * (3.3f / 4095.0f);
    
    // Tính lại điện áp thực tế của cảm biến MQ-2 (Hệ số bù 14.7 / 10 = 1.47)
    float new_smoke_v = v_at_pin * 1.47f;
    
    // Lọc nhiễu trung bình trượt (Low-pass filter): 90% cũ, 10% mới
    // Giúp khử triệt để nhiễu nhảy số khi nung nóng
    mq2_filtered_voltage = (mq2_filtered_voltage * 0.9f) + (new_smoke_v * 0.1f);
    
    // Trả về điện áp đã lọc
    return mq2_filtered_voltage;
}
