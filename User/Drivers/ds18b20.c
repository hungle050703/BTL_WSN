#include "ds18b20.h"

// Khai báo extern timer để dùng cho hàm delay_us
extern TIM_HandleTypeDef htim2;

// Hàm delay microsecond sử dụng TIM2 (Có chống treo MCU)
void delay_us(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    uint32_t timeout_cnt = 0;
    while (__HAL_TIM_GET_COUNTER(&htim2) < us) {
        timeout_cnt++;
        if (timeout_cnt > 1000000) break; // Thoát hiểm nếu TIM2 bị cấu hình sai, không nhảy số nữa
    }
}

// 1. Hàm Reset: Kiểm tra xem sensor có "sống" không
uint8_t DS18B20_Start(void) {
    uint8_t response = 0;
    
    // Kéo chân xuống thấp trong 480us (Gửi xung Reset)
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);
    
    // Thả chân lên cao và đợi 80us cho sensor phản hồi
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(80);
    
    // Đọc trạng thái chân: Nếu sensor kéo xuống thấp thì nó phản hồi (Presence)
    if (!(HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN))) {
        response = 1;
    } else {
        response = 0;
    }
    
    // Đợi nốt chu kỳ 400us
    delay_us(400); 
    
    return response;
}

// 2. Hàm Ghi 1 Byte
void DS18B20_Write(uint8_t data) {
    for (int i=0; i<8; i++) {
        if ((data & (1<<i)) != 0) { // Ghi bit 1
            // Kéo xuống thấp 1us thao tác ghi
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(1); 
            // Thả lên cao và giữ 60us cho thao tác ghi bit 1
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(60);
        } else { // Ghi bit 0
            // Kéo xuống thấp 60us cho thao tác ghi bit 0
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(60);
            // Thả lên cao và đợi 1us cho chu kỳ hồi
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(1);
        }
    }
}

// 3. Hàm Đọc 1 Byte
uint8_t DS18B20_Read(void) {
    uint8_t value = 0;
    for (int i=0; i<8; i++) {
        // Kéo chân xuống thấp 2us để bắt đầu chu kỳ đọc
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
        delay_us(2);
        
        // Thả chân lên cao (Open Drain) và đợi khoảng 10us chờ data từ sensor
        // Tối ưu timing gắt gao của DS18B20: Sau khi kéo thấp thì phải tiến hành đọc luôn vào ~15us đầu
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
        delay_us(8); 
        
        // Kiểm tra bit trả về
        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_SET) {
            value |= (1 << i);
        }
        
        // Đợi nốt chu kỳ đọc (~50us)
        delay_us(50);
    }
    return value;
}

void DS18B20_Init(void) {
    // Với chế độ Open Drain (như đã cấu hình trên CubeMX: GPIO_MODE_OUTPUT_OD),
    // chúng ta chỉ cần set ban đầu mức cao để Bus được treo bởi trở kéo lên (Pull-up).
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
}

float DS18B20_ReadTemperature(void) {
    uint8_t LSB, MSB;
    int16_t temp;
    
    if (DS18B20_Start()) { // Nếu có phản hồi từ sensor
        DS18B20_Write(0xCC); // Gửi lệnh Skip ROM (Bỏ qua địa chỉ vì dường bus chỉ đo 1 ng)
        DS18B20_Write(0x44); // Gửi lệnh Convert T (Bắt đầu đo nhiệt độ)
        
        // Cần chờ 750ms để DS18B20 chuyển đổi ADC xong (độ phân giải 12-bit mặc định).
        // Tạm thời dừng chương trình 750ms.
        HAL_Delay(750);      
        
        DS18B20_Start();     // Reset lần 2 trước khi đọc
        DS18B20_Write(0xCC); // Skip ROM
        DS18B20_Write(0xBE); // Read Scratchpad
        
        LSB = DS18B20_Read(); // Byte LSB (Nhiệt độ Byte thấp)
        MSB = DS18B20_Read(); // Byte MSB (Nhiệt độ Byte cao)
        
        // Ghép 2 byte lại
        temp = (MSB << 8) | LSB;
        return (float)temp / 16.0f; // Chia 16 vì 1 bit ứng với 0.0625°C (Theo datasheet)
    }
    
    return -999.0f; // Trả về nhiệt độ vô lí: Lỗi (Mất kết nối)
}

float DS18B20_ReadTemperature_NonBlocking(void) {
    uint8_t LSB, MSB;
    int16_t temp;
    
    if (DS18B20_Start()) { // Nếu có phản hồi từ sensor
        DS18B20_Write(0xCC); // Skip ROM
        DS18B20_Write(0xBE); // Read Scratchpad
        
        LSB = DS18B20_Read(); // Byte LSB
        MSB = DS18B20_Read(); // Byte MSB
        
        temp = (MSB << 8) | LSB;
        return (float)temp / 16.0f;
    }
    
    return -999.0f; // Lỗi (Mất kết nối)
}
