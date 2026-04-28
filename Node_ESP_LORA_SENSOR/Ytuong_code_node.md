Dưới đây là chi tiết linh kiện và hướng dẫn thiết kế Schematic cho cục **Node**.

---

### 1. Danh sách linh kiện chi tiết (cho 1 cục Node)

| **Xử lý trung tâm** : **ESP32-WROOM-32** (Smd hoặc DevKit) | Ưu điểm: Deep Sleep cực tốt (~10µA), có sẵn thư viện LoRa mạnh mẽ. |
| :--- | :--- |
| **Truyền thông RF** : **Module LoRa SX1278** (433MHz) | Giao tiếp SPI. Đây là linh kiện tiêu tốn năng lượng chính khi truyền. |
| **Cảm biến không khí** : **SHT30** hoặc **SHT31** | Giao tiếp I2C. Độ chính xác cao, dải đo rộng đúng yêu cầu đề bài. |
| **Cảm biến đất** : **Capacitive Soil Moisture v1.2** | Đầu ra Analog. Loại dung kháng không bị ăn mòn như loại điện trở. |
| **Quản lý nguồn** : **TP4056** (Module sạc Pin) | Có tích hợp bảo vệ pin (Over-discharge protection). |
| **Năng lượng** : **Pin 18650** + **Solar Panel 5V** | Tấm pin mặt trời nhỏ (60x60mm) để duy trì hoạt động lâu dài. |
| **Ổn áp** : **LDO HT7333-A** (3.3V) | Cực kỳ quan trọng: Dòng rò (Quiescent current) rất thấp, giúp pin không bị hao khi ESP32 ngủ. |

---

### 2. Hướng dẫn thiết kế Schematic (Cách nối dây)

Khi vẽ Schematic, chia thành các khối rõ:

#### A. Khối Vi điều khiển (ESP32)
* **Chân EN (Reset):** Nối trở 10kΩ lên 3.3V và tụ 100nF xuống GND để mạch khởi động ổn định.
* **Chân IO0:** Nối nút nhấn xuống GND (để nạp code khi cần).
* **Nguồn:** Cấp 3.3V vào chân 3V3 (không cấp 5V trực tiếp vào chip).

#### B. Khối LoRa SX1278 (Giao tiếp SPI)
* **MISO, MOSI, SCK:** Nối vào các chân SPI tương ứng trên ESP32.
* **NSS (Chip Select):** Nối vào một chân GPIO (thường là GPIO5).
* **RST (Reset):** Nối vào một chân GPIO (ví dụ GPIO14).
* **DIO0 (Interrupt):** **Bắt buộc nối** vào một chân GPIO (ví dụ GPIO2) để ESP32 biết khi nào việc gửi dữ liệu hoàn tất.

#### C. Khối Cảm biến (I2C & Analog)
* **SHT30:** Nối chân SCL, SDA vào cặp chân I2C của ESP32 (GPIO22, GPIO21). Thêm 2 điện trở kéo lên 4.7kΩ vào đường bus I2C.
* **Soil Sensor:** Chân Signal nối vào chân ADC của ESP32 (ví dụ GPIO34).
* **Mẹo tiết kiệm điện:** Thay vì nối VCC của cảm biến vào 3.3V trực tiếp, hãy nối nó vào một chân GPIO của ESP32. Khi ngủ, tắt chân GPIO này để cắt nguồn cảm biến hoàn toàn.

#### D. Khối Nguồn & Solar (Trọng tâm của tính bền bỉ)
* **Solar Panel:** Nối vào đầu vào của module TP4056 qua một Diode Schottky (ví dụ 1N5819) để ngăn dòng điện chạy ngược từ pin về tấm pin mặt trời ban đêm.
* **Pin 18650:** Nối vào đầu B+/B- của TP4056.
* **Ổn áp LDO:** Lấy nguồn từ Pin (khoảng 3.7V - 4.2V) đi qua HT7333 để tạo ra đúng 3.3V sạch cấp cho ESP32 và các module.

---

### 3. Các thông số "Lý thuyết"

1.  **Chế độ ngủ (Deep Sleep):** Nhờ dùng LDO HT7333 và cắt nguồn cảm biến, dòng tiêu thụ khi ngủ chỉ khoảng **15-20µA**.
2.  **Chu kỳ hoạt động (Duty Cycle):** "Thưa thầy, nút này thức dậy 2 giây để đo và gửi, sau đó ngủ 60 giây. Tỷ lệ hoạt động chỉ 3%, giúp pin 2500mAh chạy được lý thuyết hơn 1 năm".
3.  **Bảo vệ Pin:** Module TP4056 sẽ tự ngắt khi áp pin xuống dưới 2.4V, đảm bảo pin không bị hỏng (chai).

---

### 4. Ý tưởng kiến trúc Phần mềm (Software Architecture cho Node)

Đồng bộ với Gateway STM32, phần mềm trên ESP32 sẽ hoạt động theo mô hình **State Machine kết hợp Deep Sleep**.

#### 4.1. Quy trình hoạt động (Workflow)
1. **Wake-up (Thức giấc):** ESP32 thức dậy từ chế độ Deep Sleep nhờ bộ đếm thời gian.
2. **Cấp nguồn Cảm biến:** Đẩy một chân GPIO lên `HIGH` để cấp nguồn 3.3V cho cảm biến SHT30 và độ ẩm đất. Đợi 100-200ms.
3. **Đọc dữ liệu:** Đọc I2C từ SHT30 (Nhiệt độ, Độ ẩm) và ADC từ cảm biến đất.
4. **Ngắt nguồn Cảm biến:** Kéo GPIO xuống `LOW` để ngắt nguồn cảm biến.
5. **Gửi dữ liệu qua LoRa:** Khởi tạo LoRa, truyền gói tin và đợi ngắt `DIO0` (TX Done).
6. **Đưa LoRa vào chế độ Sleep:** Ghi lệnh Sleep qua SPI xuống SX1278.
7. **Deep Sleep (Ngủ sâu):** Gọi `esp_deep_sleep_start()`.

