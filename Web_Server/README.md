# 🚀 Hệ Thống Quản Lý Bãi Đỗ Xe Thông Minh (IoT Smart Parking)

Dự án Hệ thống quản lý bãi đỗ xe kết hợp thiết bị IoT (ESP32) và hệ thống Backend (Java Spring Boot), giao tiếp thời gian thực qua giao thức MQTT. Dự án giúp tự động hóa quá trình nhận diện xe ra/vào, cập nhật sơ đồ bãi đỗ và thống kê doanh thu.

## 🌟 Các Tính Năng Đã Hoàn Thiện

* 🟢 **Giám Sát Vị Trí Ô Đỗ Thực Thời:** Sử dụng cảm biến hồng ngoại (LM393) gắn tại các ô đỗ. Khi có xe vào/ra, trạng thái được đẩy qua MQTT lên Web Dashboard, đổi màu ô đỗ (Xanh: Trống / Đỏ: Có xe) ngay lập tức.
* 💳 **Kiểm Soát Ra/Vào Bằng 1 Đầu Đọc RFID (Tối Ưu Phần Cứng):** Sử dụng duy nhất 1 module RC522 để quản lý cả 2 làn (Vào và Ra) dựa vào trí nhớ Database:
  * **Chưa có trong bãi:** Tự động ghi nhận là xe VÀO -> Mở Barie vào.
  * **Đã có trong bãi:** Tự động ghi nhận là xe RA -> Tính tiền -> Lưu lịch sử -> Mở Barie ra.
* 🚧 **Điều Khiển Barie Khẩn Cấp (Manual Toggle):** Cho phép bảo vệ Đóng/Mở Barie Đường Vào và Đường Ra trực tiếp từ giao diện Web thông qua các mã lệnh khẩn cấp (O1/C1, O2/C2) gửi thẳng xuống vi điều khiển.
* 📊 **Web Dashboard Quản Lý:** * Cập nhật sơ đồ bãi đỗ tự động.
  * Hiển thị bảng lịch sử quẹt thẻ ra/vào liên tục.
  * Biểu đồ doanh thu tự động vẽ dựa trên dữ liệu tính phí thực tế.
* 🔐 **Quản Lý Thẻ Hợp Lệ:** Thêm mới thẻ RFID, xem danh sách thẻ và xóa thẻ không hợp lệ khỏi hệ thống.

## 🛠️ Công Nghệ Sử Dụng

* **Backend:** Java Spring Boot, Spring Data JPA, Hibernate.
* **Database:** MySQL.
* **Frontend:** HTML, CSS, JavaScript (Vanilla), Chart.js (vẽ biểu đồ).
* **Giao thức:** MQTT (Sử dụng thư viện Eclipse Paho MQTT cho Java) & HTTP RESTful API.
* **Phần Cứng (Nhúng):** Vi điều khiển ESP32, Đầu đọc thẻ RC522, Cảm biến hồng ngoại LM393, Động cơ Servo SG90 (giả lập Barie).

## 📡 Sơ Đồ Quy Hoạch Kênh MQTT (Topic Architecture)

Dự án sử dụng Broker trung gian (`broker.emqx.io`) với các kênh được chia luồng nghiêm ngặt:

**1. Kênh Nhận Tín Hiệu (Từ ESP32 ném lên Java):**
* `👉 /state/park/#`: Kênh báo trạng thái ô đỗ từ cảm biến.
* `👉 /ptit/parking/check`: Kênh gửi mã thẻ UID khi có người quẹt thẻ vật lý.

**2. Kênh Gửi Lệnh (Từ Java điều khiển ESP32):**
* `👈 /ptit/servo/control`: Gửi tín hiệu mở cổng tự động sau khi quét thẻ hợp lệ (`1`: Mở cổng Vào, `2`: Mở cổng Ra).
* `👈 /ptit/servo/emergency`: Gửi tín hiệu Đóng/Mở cổng thủ công từ bảo vệ (`O1/C1`: Mở/Đóng cổng Vào, `O2/C2`: Mở/Đóng cổng Ra).

## ⚙️ Hướng Dẫn Cài Đặt 

### 1. Khởi chạy Backend & Web
1. Import project vào IDE (IntelliJ IDEA / Eclipse).
2. Tạo database tên là `baidoxe` trong MySQL.
3. Cài đặt thông tin kết nối Database trong file `application.properties`:
   ```properties
   spring.datasource.url=jdbc:mysql://localhost:3306/baidoxe
   spring.datasource.username=root
   spring.datasource.password=mật_khẩu_của_bạn
