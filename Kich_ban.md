# BTL_WSN
KỊCH BẢN BTL MẠNG CẢM BIẾN KHÔNG DÂY

1. Tổng quan hệ thống (Trông như thế nào?)
Các "Trạm thu tin" (Nút cảm biến): Là những chiếc hộp nhỏ cắm rải rác khắp cánh đồng. Mỗi hộp có một chân cắm xuống đất để đo độ ẩm, và một cảm biến bên trên để đo nhiệt độ và độ ẩm không khí xem trời nóng hay ẩm. Chạy bằng pin kèm thêm pin mặt trời
"Bộ xử lý trung tâm" (Gateway): Là một chiếc hộp lớn hơn, đặt ở đầu làng hoặc nhà kho. Có nhiệm vụ nghe ngóng tín hiệu từ tất cả các trạm nhỏ gửi về.
"Xuất dữ liệu” (Phần mềm): Là màn hình máy tính, nơi mọi thông tin từ cánh đồng hiện ra dưới dạng bảng biểu và có thể xuất ra excel.

2. Cơ chế hoạt động (chạy như thế nào?)
Theo quy trình "Ngủ - Thức - Báo cáo":
Chế độ tiết kiệm: Để hoạt động lâu dài, các trạm nhỏ phần lớn thời gian sẽ "ngủ" để tiết kiệm pin).
Báo cáo nhanh: Cứ mỗi vài phút, nó đo các chỉ số nhiệt độ, độ ẩm rồi gửi qua sóng radio (giao thức Lora): 
Tiếp nhận và Chuyển phát: Bộ xử lý trung tâm (Gateway) luôn thức 24/7. Khi bắt tín hiệu từ bất kỳ trạm nào, nó sẽ ghi lại ngay lập tức và dùng sóng điện thoại (4G) để nhắn tin real time lên mạng Internet.
Hiển thị: nhìn lên màn hình máy tính các thông báo 
SƠ ĐỒ LINH KIỆN
1. Sơ đồ kiến trúc tổng thể (System Overview)
Tầng 1: Sensor Nodes (02 Nút thực tế - Đặt tại các điểm khác nhau của cánh đồng (theo yêu cầu lý thuyết là 100-1000 nút nhưng làm thật phần cứng chỉ 2 nút)
Thành phần mỗi nút: ESP32 + LoRa SX1278 + Cảm biến SHT30 + Pin & Solar.
Cơ chế: Ngủ sâu (Deep Sleep) → Thức dậy → Đọc cảm biến → Gửi LoRa → Chờ ACK → Ngủ tiếp.
Nhiệm vụ: "Mắt xích" thu thập dữ liệu tại mỗi nút.
Tầng 2: Gateway (01 Bộ trung tâm - Đặt tại trạm điều hành (nhận dữ liệu đo từ tất cả các nút)
Thành phần: STM32H563 (Xử lý chính) + LoRa SX1278 (Nhận tin) + SIMCOM A7677 (Đẩy tin).
Cơ chế: Lắng nghe tất cả các ID nút → Phân loại dữ liệu → Đóng gói giao thức MQTT → Đẩy qua mạng 4G.
Nhiệm vụ: "Cầu nối" chuyển đổi từ sóng Radio nội bộ sang Internet toàn cầu.
Tầng 3: Monitoring & Cloud (Phần mềm máy tính)
Thành phần: MQTT Broker + Dashboard (C# hoặc Web) + Database (MySQL/Excel).
Nhiệm vụ: Hiển thị biểu đồ real-time, lưu trữ và xuất báo cáo dạng excel.

2. Mô hình lắp đặt vật lý (Physical Look)
thiết bị sẽ trông như thế này:
Thành phần
Hình dáng bên ngoài
Cách triển khai
Nút
Một hộp nhựa kín, trên đỉnh có tấm pin Solar, một anten râu, phía dưới thò ra đầu cảm biến độ ẩm cắm xuống đất.
Cắm trực tiếp tại các vị trí cần giám sát nhiệt độ, độ ẩm trên cánh đồng.
Gatewa4Gy
Một hộp lớn hơn, có anten LoRa dài và anten 4G, bên trong có board STM32H5 kết nối dây với module SIM.
Đặt ở nơi có sóng 4G tốt (cột điện hoặc mái hiên nhà kho).

Thiết bị nhận 4G

3. Luồng dữ liệu Real-time (Data Flow)
Để chứng minh tính Real-time khi quản lý từ 100 nút, hãy nhìn vào luồng xử lý này:
T=0s: Node 1 đọc được nhiệt độ 35∘C.
T=0.1s: Node 1 gửi gói tin LoRa (ID=01, Temp=35).
T=0.2s: STM32H5 nhận tin, xác thực ID, nháy LED báo hiệu nhận thành công.
T=0.5s: STM32H5 đẩy lệnh AT Command qua A7677 gửi gói tin MQTT lên Server.
T=1.5s: Dashboard trên máy tính nhận được gói tin, biểu đồ nhảy số ngay lập tức.

4. Giải pháp "Giả lập 100 nút" (The Trick)
Đây là điểm để thuyết phục về tính khả thi:
Thực tế: Nhóm chỉ làm 2 mạch Node thật.
Phần mềm: Trong code của Node 2, cứ mỗi 30 giây, nó sẽ gửi 10 gói tin liên tiếp nhưng đổi Node_ID từ 1 đến 10.
Kết quả: Trên Dashboard sẽ hiện ra danh sách 10 vị trí khác nhau đang gửi dữ liệu về. Điều này chứng minh thuật toán của nhóm quản lý được danh sách nút lớn mà không cần mua 100 bộ linh kiện.

LINH KIỆN CẦN CHUẨN BỊ 
1. Danh sách linh kiện đã có (Already Have)
01 Board STM32H563 (Nucleo-H563ZI hoặc tương đương): Đóng vai trò là bộ não của Gateway.
01 Module SIMCOM A7677S: Kết nối Internet qua 4G (đẩy dữ liệu lên Server).

2. Danh sách linh kiện cần mua thêm (Shopping List)
A. Cho Gateway (Để hoàn thiện bộ thu)
01 Module LoRa SX1276 (hoặc SX1278 - 433MHz): Để STM32H563 có thể giao tiếp không dây với các Node.
Lưu ý: Chọn loại có giao tiếp SPI để dễ kết nối với STM32.
01 Ăng-ten LoRa (433MHz/915MHz tùy module): Loại có đế nam châm hoặc gắn trực tiếp.
B. Cho 02 Node cảm biến (Nút lá)
02 Vi điều khiển Low-power: Khuyên dùng ESP32 (Dễ làm OTA, Deep Sleep tốt) hoặc STM32L151 (Siêu tiết kiệm điện). Nếu muốn đồng bộ với Gateway, hãy chọn STM32 BluePill (F103) nhưng sẽ tốn công tối ưu pin hơn.
02 Module LoRa SX1276/SX1278: Phải cùng loại và cùng tần số với module ở Gateway.
02 Cảm biến Nhiệt độ & Độ ẩm không khí SHT30/SHT31: Chuẩn I2C, có vỏ bọc chống nước (SHT30-DIS).
02 Cảm biến Độ ẩm đất dung kháng (Capacitive Soil Moisture Sensor v1.2): Loại màu đen, chống ăn mòn.
02 Bộ nguồn:
02 Pin 18650 (Dung lượng ~2500mAh - 3000mAh).
02 Đế giữ pin 18650.
02 Mạch sạc & Bảo vệ pin (TP4056 hoặc mạch tích hợp Solar CN3791).
02 Tấm pin năng lượng mặt trời mini (5V - 100mA đến 200mA).
C. Phụ kiện lắp đặt (Đóng gói IP68)
Board đục lỗ (Matrix Board) hoặc đặt PCB: Để hàn mạch cố định (tránh dùng Breadboard vì sẽ rất nhiễu khi truyền RF).
3. Bảng tổng hợp linh kiện cần mua (Tóm tắt)
Nhóm
Linh kiện
Số lượng
Ước tính chi phí
Truyền thông
Module LoRa SX1278 (SPI)
3
~180.000đ
Xử lý
ESP32 (mỗi Node cần 1 con)
2
~120.000đ
Cảm biến
SHT30 + Độ ẩm đất v1.2 (Mỗi node cần một bộ cảm biến này)
2 bộ
~200.000đ
Nguồn
Pin 18650 + Sạc + Solar (mỗi node một bộ)
2 bộ
~150.000đ








Tổng cộng




~650.000đ



