#include "sensor_service.h"
#include "ds18b20.h"
#include "mq2.h"

// Biến buffer toàn cục cho DMA (Đọc 1 kênh ADC)
uint16_t adc_dma_buffer[1];
extern ADC_HandleTypeDef hadc1;

static uint32_t last_ds18b20_tick = 0;
static uint8_t ds18b20_state = 0; // 0: Idle/Request, 1: Waiting for conversion

void SensorService_Init(void) {
    DS18B20_Init();
    MQ2_Init();
    // Bắt chẩn đoán: Tạm tắt DMA vì nghi ngờ hadc1.DMA_Handle đang bị NULL gây HardFault
    // HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 1);
}

void SensorService_Update(SensorData_t *data) {
    uint32_t current_tick = HAL_GetTick();

    // --- Xử lý DS18B20 không dùng Delay (Non-blocking) ---
    if (ds18b20_state == 0) {
        // Giai đoạn 1: Ra lệnh cho DS18B20 bắt đầu đo
        if (DS18B20_Start()) {
            DS18B20_Write(0xCC); // Skip ROM
            DS18B20_Write(0x44); // Gửi lệnh Convert T
            last_ds18b20_tick = current_tick;
            ds18b20_state = 1;   // Chuyển sang trạng thái chờ
        }
    } 
    else if (ds18b20_state == 1) {
        // Giai đoạn 2: Kiểm tra xem đã đủ 750ms chưa
        if (current_tick - last_ds18b20_tick >= 750) {
            data->temperature = DS18B20_ReadTemperature_NonBlocking();
            ds18b20_state = 0;   // Quay lại chu kỳ tiếp theo
        }
    }
    
    // --- Xử lý MQ2 từ buffer DMA (Dữ liệu luôn sẵn sàng) ---
    data->smoke_raw = MQ2_ReadRawData();
    data->smoke_conc = MQ2_CalculateGasConcentration(data->smoke_raw);
}
