#include "mq2.h"

// Khai báo extern mảng chứa dữ liệu ADC dùng DMA (sẽ định nghĩa trong main.c hoặc sensor_service.c)
extern uint16_t adc_dma_buffer[1]; 

void MQ2_Init(void) {
    // Khởi tạo MQ2 nếu cần thiết (ví dụ bật heater, hiệu chuẩn ban đầu)
}

uint32_t MQ2_ReadRawData(void) {
    // Vì dùng DMA vòng lặp nên giá trị adc_dma_buffer luôn được tự động cập nhật
    return adc_dma_buffer[0];
}

float MQ2_CalculateGasConcentration(uint32_t raw_adc) {
    // Chuyển đổi mã ADC thô sang giá trị điện áp hoặc % nồng độ khí/khói
    float voltage = (float)raw_adc * (3.3f / 4095.0f);
    // Tính toán theo datasheet MQ2 (cần hiệu chuẩn)
    return voltage; // Tạm thời trả về điện áp
}
