
Danh sách linh kiện cần thiết để vẽ Schematic cho phần cứng Gateway:

---

### 1. Khối Xử lý trung tâm (Brain Control)
Đây là "trái tim" điều phối mọi hoạt động từ nhận sóng LoRa đến đẩy dữ liệu lên 4G.
* **MCU:** STM32H563 (Dòng Cortex-M33, xung nhịp cao, hỗ trợ bảo mật tốt).
* **Thạch anh (Crystal):** 24MHz hoặc 25MHz (cho HSE) và 32.768kHz (cho LSE/RTC).
* **Tụ lọc nguồn (Decoupling Capacitors):** Các tụ $100nF$ và $4.7\mu F$ đặt sát các chân VDD/VSS của chip.
* **Mạch Reset:** Điện trở $10k\Omega$ kéo lên và tụ $100nF$ nối đất.
* **Cổng nạp/Debug:** Header 4-pin (VCC, GND, SWDIO, SWCLK) để kết nối với mạch nạp ST-Link.

### 2. Khối Truyền thông LoRa (RF Layer)
Để giao tiếp với các nút cảm biến ngoài cánh đồng.
* **Module LoRa:** SX1278 (hoặc SX1276) giao tiếp qua chuẩn **SPI**.
* **Linh kiện hỗ trợ:**
    * Điện trở kéo lên cho chân `NSS` (Chip Select).
    * Kết nối các chân `DIO0`, `DIO1` vào chân ngắt (Interrupt) của STM32 để nhận biết khi có gói tin đến.
* **Ăng-ten:** Đầu nối SMA hoặc IPEX để gắn ăng-ten 433MHz.

### 3. Khối truyền thông 4G (Internet Layer)
Để đẩy dữ liệu lên Cloud.
* **Module:** SIMCOM A7677S.
* **Linh kiện quan trọng:**
    * **Khe cắm SIM:** Nano SIM hoặc Micro SIM slot.
    * **Mạch bảo vệ SIM:** Chip TVS (như SMF05C) để chống tĩnh điện cho khe SIM.
    * **Transistor điều khiển:** Để đóng/ngắt nguồn module hoặc điều khiển chân `PWRKEY` (bật nguồn module bằng phần mềm).
    * **Anten 4G:** Đầu nối IPEX/SMA cho ăng-ten LTE.



### 4. Khối Quản lý nguồn (Power Management)
Gateway cần nguồn cực kỳ ổn định vì module 4G khi truyền tin có thể ngốn dòng tức thời lên tới $2A$.
* **Đầu vào:** Jack DC 9V-12V hoặc Terminal block.
* **Hạ áp cấp cho STM32 & LoRa (3.3V):** IC nguồn LDO (như AMS1117-3.3) hoặc mạch Buck (như LM2596-3.3) nếu muốn hiệu suất cao.
* **Hạ áp cấp cho A7677S (3.8V - 4.2V):** Module 4G cần dải áp riêng. Thường dùng IC Buck dòng lớn (như LM2596-ADJ hoặc MIC29302).
* **Tụ lưu trữ:** Một tụ điện phân lớn ($1000\mu F$ hoặc hơn) đặt gần chân nguồn của module 4G để bù dòng tức thời.

### 5. Các khối bổ sung (Interface & Debug)
* **Đèn LED báo hiệu:** * LED Power (Nguồn).
    * LED Status (Trạng thái mạng 4G - lấy từ chân `NETLIGHT` của A7677).
    * LED Data (Nháy khi có dữ liệu LoRa gửi về).
* **Khe cắm thẻ nhớ MicroSD:** Giao tiếp qua **SDIO** của STM32 (để lưu trữ dữ liệu dự phòng khi mất mạng 4G).
* **Nút nhấn:** 01 nút Reset, 01 nút User (để cấu hình nhanh).
* **UART to USB:** Chip CP2102 hoặc CH340 (để bạn cắm vào laptop xem Log dữ liệu trực tiếp).

---

### Lưu ý quan trọng khi vẽ Schematic:
1.  **Chống nhiễu:** Đường tín hiệu từ chân Ăng-ten LoRa và 4G ra đầu nối cần vẽ ngắn nhất có thể và bọc đồng (Ground plane) xung quanh kỹ lưỡng.
2.  **Độ rộng đường dây nguồn:** Đường cấp nguồn cho module 4G phải vẽ rất to (ít nhất 40-60 mil) để đảm bảo không bị sụt áp khi module truyền dữ liệu.
3.  **Mức điện áp (Voltage Level):** Hãy kiểm tra xem module A7677 bạn dùng có mức logic là 1.8V hay 3.3V. Nếu là 1.8V, bạn cần thêm IC chuyển đổi mức logic (Level Shifter) khi nối với STM32H5 (vốn chạy 3.3V).



Sơ đồ nối dây "Lý thuyết" 



    Khối Nguồn: Adapter 12V → Module hạ áp LM2596 (Ra 4V cho 4G và 3.3V cho STM32).

    Khối Xử lý: STM32H5 nối với LoRa qua SPI (Trực tiếp 3.3V).

    Khối 4G: STM32H5 nối với A7677 qua UART (Thông qua module chuyển mức logic).

    Khối Lưu trữ: Thẻ nhớ MicroSD nối qua SPI/SDIO.