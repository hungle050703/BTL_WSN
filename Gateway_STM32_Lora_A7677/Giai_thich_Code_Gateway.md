# GIẢI THÍCH KIẾN TRÚC VÀ MÃ NGUỒN CỦA GATEWAY (STM32H5)

Tài liệu này giải thích chi tiết cách thức hoạt động của phần mềm trên vi điều khiển trung tâm (Gateway STM32H563) trong hệ thống Mạng cảm biến không dây (WSN).

## 1. TỔNG QUAN KIẾN TRÚC PHẦN MỀM
Gateway được thiết kế chạy trên hệ điều hành thời gian thực **FreeRTOS**. Kiến trúc được chia làm 3 bộ phận chính hoạt động song song để đảm bảo tính Real-time (Thời gian thực) và không làm mất/rớt gói tin của các nút mạng:
- **Ngắt phần cứng (EXTI):** Bắt tín hiệu cực nhanh khi có sóng vô tuyến bay tới.
- **Hàng đợi tin nhắn (Message Queue):** Một "đường ống" lưu trữ đệm giữa việc nhận (LoRa) và đẩy mạng (SIM 4G).
- **Luồng xử lý (Tasks):** 02 Luồng chạy song song phụ trách 2 ngoại vi riêng biệt.

---

## 2. CHI TIẾT CÁC TẦNG XỬ LÝ (Nằm trong `app_freertos.c` và `main.c`)

### 2.1. Tầng 1: Xử lý Ngắt phần cứng (Hardware Interrupt)
**Vị trí code:** Cuối file `main.c` (`HAL_GPIO_EXTI_Callback`).
**Nguyên lý:**
Khi module LoRa SX1278 nhận được một tín hiệu Radio hợp lệ từ Node ESP32, nó sẽ bật điện áp ở chân `LORA_DIO0_Pin`. Lập tức, STM32 nhảy vào hàm ngắt (dù đang làm gì ở hàm main cũng phải dừng lại).
Trong hàm ngắt, ta KHÔNG đọc dữ liệu ngay (vì sẽ làm treo chip), mà ta nhấc một thẻ gọi là **Semaphore** (`Lora_Rx_SemHandle`) lên rồi thoát hàm cái rụp.
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == LORA_DIO0_Pin) {
    osSemaphoreRelease(Lora_Rx_SemHandle); // Đánh thức luồng LoRa
  }
}
```

### 2.2. Tầng 2: Luồng thu thập (Lora_Rx_Task)
**Vị trí code:** File `app_freertos.c`.
**Nguyên lý:**
- Luồng này khởi tạo thông số LoRa (`SX1278_Init()`), cấu hình "Quy tắc vàng 5 món" (Tần số 433MHz, SF7, BW125kHz, SyncWord 0x12) để **tương thích 100% với mạch ESP32 Node**.
- Trong vòng lặp vô tận, nó dùng hàm `osSemaphoreAcquire` để **ĐI NGỦ (Block status)**. Nó không tốn 1% CPU nào bảo vệ vòng lặp.
- Hễ Tầng 1 (hàm ngắt) dựng cờ Semaphore, nó lập tức TỈNH DẬY.
- Thức dậy xong, nó mượn hệ điều hành 32-byte RAM (`pvPortMalloc(32)`), tải mảng Byte từ chip LoRa thông qua đường SPI đút vào mảng RAM đó. 
- Cuối cùng, nó ghi địa chỉ của mảng RAM vào đường ống truyền tin `SensorDataQueue` để luồng 4G xử lý tĩnh. Rồi lại Đi Ngủ chờ sóng tiếp theo.

### 2.3. Tầng 3: Luồng xuất bản (Sim4G_Tx_Task)
**Vị trí code:** Phía dưới Lora_Rx_Task trong `app_freertos.c`.
**Nguyên lý:**
- Nhiệm vụ duy nhất: Ngồi rình ngay đầu ra của đường ống `SensorDataQueue`. 
- Nó dùng lệnh `osMessageQueueGet` để nằm chầu ở đó chờ. Khi Luồng 2 (LoRa) ném địa chỉ mảng RAM vào, nó vớt ra.
- Nó gọi hàm điều khiển `A7677_SendSMS` (hoặc sau này là MQTT) và quăng thẳng chuỗi Text Raw đó lên không gian mạng 4G.
- Khi hoàn tất việc gửi chui lệnh AT qua UART5, nó dùng `vPortFree(txString)` để xoá sạch đoạn RAM đó lấy không gian dư dả cho các Nút số 2, số 3... sau này gửi lên.

---

## 3. THƯ VIỆN LORA SX1278 (`sx1278.c` / `.h`)
- Được Port (chuyển thể) trực tiếp từ cấu trúc Register của Semtech và tham khảo sát với lõi LoRa Sandeep Mistry của hệ sinh thái Arduino.
- Đảm bảo **sóng của ESP32 và STM32 chạm nhau** mà không gặp hiện tượng mã hoá sai.
- Chế độ ngầm định: `SX1278_Receive(0)` được gọi để mạch chuyển vào trạng thái RX_CONTINUOUS (Lắng nghe liên tục) thay vì Receive_Single.

## 4. TAI SAO LẠI PHẢI DÙNG CON TRỎ QUA QUEUE? (Memory Strategy)
Nếu cấu hình Queue có `ItemSize = 32 bytes` (Kích cỡ mỗi tin là 32 kí tự text), mỗi lần qua ống ta phải "copy - paste" toàn bộ 32 kí tự này. Điều này làm lãng phí RAM và giảm tốc độ băng thông của RTOS.
Bằng cách cấp phép `pvPortMalloc` cho một mảng, và chỉ nhét ĐỊA CHỈ (Pointer - nhẹ đúng 4 Bytes) của mảng đó vào Queue, Hệ thống xử lý đa luồng diễn ra với tốc độ vi giây cực kỳ tối ưu cho Chip nhúng.