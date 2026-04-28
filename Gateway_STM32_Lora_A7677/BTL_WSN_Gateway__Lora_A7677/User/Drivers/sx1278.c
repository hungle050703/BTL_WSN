#include "sx1278.h"

// Define helper
#define SX1278_CS_LOW()  HAL_GPIO_WritePin(LORA_NSS_PORT, LORA_NSS_PIN, GPIO_PIN_RESET)
#define SX1278_CS_HIGH() HAL_GPIO_WritePin(LORA_NSS_PORT, LORA_NSS_PIN, GPIO_PIN_SET)
#define SX1278_RST_LOW()  HAL_GPIO_WritePin(LORA_RST_PORT, LORA_RST_PIN, GPIO_PIN_RESET)
#define SX1278_RST_HIGH() HAL_GPIO_WritePin(LORA_RST_PORT, LORA_RST_PIN, GPIO_PIN_SET)

uint8_t SX1278_ReadRegister(uint8_t reg) {
    uint8_t tx = reg & 0x7F;
    uint8_t rx = 0;
    SX1278_CS_LOW();
    HAL_SPI_Transmit(SX1278_SPI, &tx, 1, 1000);
    HAL_SPI_Receive(SX1278_SPI, &rx, 1, 1000);
    SX1278_CS_HIGH();
    return rx;
}

void SX1278_WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg | 0x80;
    buf[1] = value;
    SX1278_CS_LOW();
    HAL_SPI_Transmit(SX1278_SPI, buf, 2, 1000);
    SX1278_CS_HIGH();
}

int SX1278_Begin(long frequency) {
    // Reset vi mạch
    SX1278_RST_LOW();
    HAL_Delay(10);
    SX1278_RST_HIGH();
    HAL_Delay(10);
    
    // Check version
    uint8_t version = SX1278_ReadRegister(REG_VERSION);
    if (version != 0x12) {
        return 0; // Thất bại (ko nhận ra lora)
    }
    
    // Setup Sleep mode
    SX1278_Sleep();
    // Default Lora Frequency
    SX1278_SetFrequency(frequency);
    
    // Đặt tham số cơ bản theo thư viện Lora ESP32 (SandeepMistry)
    SX1278_WriteRegister(REG_FIFO_TX_BASE_ADDR, 0);
    SX1278_WriteRegister(REG_FIFO_RX_BASE_ADDR, 0);
    SX1278_WriteRegister(REG_LNA, SX1278_ReadRegister(REG_LNA) | 0x03); 
    SX1278_WriteRegister(REG_MODEM_CONFIG_3, 0x04);
    SX1278_SetTxPower(17);
    SX1278_Idle(); // Chuyển sang standby mờ
    return 1; // Khởi tạo thành công
}

void SX1278_SetFrequency(long frequency) {
    uint32_t frf = ((uint64_t)frequency << 19) / 32000000;
    SX1278_WriteRegister(REG_FRF_MSB, (uint8_t)(frf >> 16));
    SX1278_WriteRegister(REG_FRF_MID, (uint8_t)(frf >> 8));
    SX1278_WriteRegister(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

void SX1278_Sleep(void) {
    SX1278_WriteRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void SX1278_Idle(void) {
    SX1278_WriteRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void SX1278_SetTxPower(int level) {
    if (level > 17) {
        if (level > 20) level = 20;
        level -= 3;
        SX1278_WriteRegister(REG_PA_CONFIG, 0x80 | (level - 2));
    } else {
        SX1278_WriteRegister(REG_PA_CONFIG, 0x80 | (level - 2));
    }
}

void SX1278_SetSpreadingFactor(int sf) {
    if (sf < 6) sf = 6; else if (sf > 12) sf = 12;
    if (sf == 6) {
        SX1278_WriteRegister(REG_MODEM_CONFIG_2, (SX1278_ReadRegister(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) | 0x04));
    } else {
        SX1278_WriteRegister(REG_MODEM_CONFIG_2, (SX1278_ReadRegister(REG_MODEM_CONFIG_2) & 0x0f) | (sf << 4));
    }
}

void SX1278_SetSignalBandwidth(long sbw) {
    int bw;
    if (sbw <= 7.8E3) bw = 0; else if (sbw <= 10.4E3) bw = 1; else if (sbw <= 15.6E3) bw = 2; else if (sbw <= 20.8E3) bw = 3; else if (sbw <= 31.25E3) bw = 4; else if (sbw <= 41.7E3) bw = 5; else if (sbw <= 62.5E3) bw = 6; else if (sbw <= 125E3) bw = 7; else if (sbw <= 250E3) bw = 8; else bw = 9;
    SX1278_WriteRegister(REG_MODEM_CONFIG_1, (SX1278_ReadRegister(REG_MODEM_CONFIG_1) & 0x0f) | (bw << 4));
}

void SX1278_SetCodingRate4(int denominator) {
    int cr = denominator - 4;
    SX1278_WriteRegister(REG_MODEM_CONFIG_1, (SX1278_ReadRegister(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
}

void SX1278_SetPreambleLength(long length) {
    SX1278_WriteRegister(REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
    SX1278_WriteRegister(REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

void SX1278_SetSyncWord(int sw) {
    SX1278_WriteRegister(REG_SYNC_WORD, sw);
}

void SX1278_Receive(int size) {
    SX1278_WriteRegister(REG_DIO_MAPPING_1, 0x00);
    if(size > 0) {
        SX1278_WriteRegister(REG_PAYLOAD_LENGTH, size);
    }
    SX1278_WriteRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

int SX1278_ParsePacket(int size) {
    // Check IRQ
    int irqFlags = SX1278_ReadRegister(REG_IRQ_FLAGS);
    SX1278_WriteRegister(REG_IRQ_FLAGS, irqFlags); // Clear cờ ngắt
    int packetLength = 0;
    
    // Có cờ RX Done 
    if ((irqFlags & IRQ_RX_DONE_MASK) && ((irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0)) {
        packetLength = SX1278_ReadRegister(REG_RX_NB_BYTES);
        // Trỏ con trỏ đọc FIFO về đúng vị trí đầu gói tin
        SX1278_WriteRegister(REG_FIFO_ADDR_PTR, SX1278_ReadRegister(REG_FIFO_RX_CURRENT_ADDR));
        // Idle lại để sẵn sàng xử lý
        SX1278_Idle();
    }
    return packetLength;
}

uint8_t SX1278_Read(void) {
    return SX1278_ReadRegister(REG_FIFO);
}

void SX1278_ReadBuffer(uint8_t *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = SX1278_ReadRegister(REG_FIFO);
    }
}

// Gọi hàm này sau khi cấu hình xong để lắng nghe liên tục liên tục
void SX1278_Init(void) {
    // 433 MHz là mặc định
    SX1278_Begin(433000000); 
    SX1278_SetSyncWord(0x12);    // Mạng nội bộ 0x12 (như ESP32)
    SX1278_SetSpreadingFactor(7); // SF7 
    SX1278_SetCodingRate4(5);     // CR4/5
    SX1278_SetSignalBandwidth(125E3); // BW 125kHz
    SX1278_SetPreambleLength(8);
    // Đưa vào chế độ nghe liên tục chờ tín hiệu Nút
    SX1278_Receive(0); 
}