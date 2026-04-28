# Ý tưởng lập trình Firmware cho Node (ESP32 + LoRa)

Để tối ưu hóa năng lượng và đồng bộ với Gateway STM32, phần mềm trên ESP32 (lập trình qua Arduino IDE/PlatformIO) sẽ hoạt động theo mô hình **State Machine kết hợp Deep Sleep**.

## 1. Quy trình hoạt động (Workflow)
Quy trình của một Node diễn ra theo chu kỳ cực kỳ ngắn và nhanh để tiết kiệm pin tối đa:
1. **Wake-up (Thức giấc):** ESP32 thức dậy từ chế độ Deep Sleep nhờ bộ đếm thời gian (RTC Timer) bên trong (ví dụ: mỗi 5-10 phút 1 lần).
2. **Cấp nguồn Cảm biến (Power up sensors):** Đẩy một chân GPIO lên `HIGH` để đóng Mosfet/Transistor cấp nguồn 3.3V cho cảm biến SHT30 và độ ẩm đất. Đợi khoảng 100-200ms để cảm biến ổn định.
3. **Đọc dữ liệu (Data Acquisition):** 
   - Đọc I2C từ SHT30 (Nhiệt độ, Độ ẩm không khí).
   - Đọc ADC từ cảm biến độ ẩm đất (chuyển đổi dải điện áp sang phần trăm 0-100%).
4. **Ngắt nguồn Cảm biến (Power down sensors):** Ngay lập tức kéo GPIO xuống `LOW` để cắt nguồn cảm biến, tránh lãng phí dòng tĩnh.
5. **Đóng gói Dữ liệu (Payload Formatting):** Gom dữ liệu vào một chuỗi (String) hoặc Struct (ví dụ: `NodeID=1,Temp=25.4,Hum=60.2,Soil=45`). Đóng gói ngắn ngọn để thời gian phát sóng (Time-on-Air) của LoRa là thấp nhất.
6. **Gửi dữ liệu qua LoRa (RF Transmission):**
   - Khởi tạo LoRa (bật nguồn cho module SX1278).
   - Truyền gói tin. ESP32 sẽ chờ ngắt từ chân `DIO0` (TX Done) để xác nhận việc gửi đã đẩy ra không trung hoàn tất.
7. **Đưa LoRa vào chế độ Sleep:** Ghi lệnh Sleep qua SPI xuống SX1278 (tiêu thụ ~1uA).
8. **Deep Sleep (Ngủ sâu):** ESP32 gọi hàm `esp_deep_sleep_start()` để ngủ hoàn toàn, chỉ giữ lại RTC hoạt động. Quay lại bước 1.

## 2. Đồng bộ tham số LoRa với Gateway STM32
Đây là yếu tố then chốt nhất để Node có thể nói chuyện được với Gateway mà chúng ta đã cấu hình bằng CubeMX lúc trước. Thư viện khuyên dùng: `LoRa by Sandeep Mistry` trên Arduino IDE. 
Cấu hình bắt buộc phải khớp 100% với Gateway:
*   **Tần số (Frequency):** 433E6 (433 MHz)
*   **Spreading Factor (SF):** 7 (Cân bằng giữa tốc độ và khoảng cách)
*   **Signal Bandwidth (BW):** 125E3 (125 kHz)
*   **Coding Rate (CR):** 4/5 
*   **Sync Word:** 0x12 (Để không bị nhiễu bởi các mạng LoRa khác xung quanh)

## 3. Xử lý "Đụng độ" (Collision Avoidance - Nâng cao)
Vì chúng ta có nhiều Node truyền về 1 Gateway (Kiến trúc mạng hình Sao - Star Topology), nếu 2 Node cùng thức dậy và phát cùng lúc, sóng sẽ bị nhiễu và vỡ gói tin (Collision).
*   **Giải pháp (CSMA cơ bản):** Trước khi phát, Node dùng chế độ CAD (Channel Activity Detection) của SX1278 hoặc nhận RSSI để "nghe" xem kênh có đang trống không. Nếu có người đang phát, nó sẽ delay random (ví dụ 100-500ms) rồi thử lại.
*   **Giải pháp Time-Slot:** Kèm thêm 1 offset thời gian ngủ ngẫu nhiên (Sleep Time + Random(0, 5) seconds) để các Node tự trượt chu kỳ thức giấc ra xa nhau.

