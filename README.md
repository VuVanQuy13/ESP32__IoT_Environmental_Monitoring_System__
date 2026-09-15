# ESP32 Environmental Monitor — FreeRTOS

Hệ thống giám sát và điều khiển môi trường sử dụng ESP32: đo nhiệt độ, độ ẩm và bụi; hiển thị dữ liệu qua web/Blynk và điều khiển ba ngõ ra theo chế độ tự động hoặc thủ công.

## Chức năng

- Đọc nhiệt độ, độ ẩm từ DHT11 và mức bụi từ Sharp GP2Y10.
- Hiển thị số đo, điều chỉnh ngưỡng qua giao diện web và Blynk.
- Chế độ **Auto** điều khiển theo ngưỡng; **Manual** điều khiển từng ngõ ra qua Blynk.
- Lưu ngưỡng bằng Preferences/NVS; thử kết nối lại Wi-Fi và Blynk khi mất mạng.
- Tổ chức firmware thành ba task FreeRTOS: **Sensor**, **Control**, **Communication**, trao đổi dữ liệu qua queue và theo dõi bằng watchdog.

## Phần cứng

| Thành phần | Vai trò / kết nối |
| --- | --- |
| ESP32 DOIT DevKit V1 | Vi điều khiển trung tâm, kết nối Wi-Fi |
| DHT11 | Đo nhiệt độ, độ ẩm — GPIO13 |
| Sharp GP2Y10 | Đo bụi — ADC GPIO34, điều khiển LED GPIO25 |
| Ba ngõ ra điều khiển | GPIO22: nhiệt độ; GPIO5: độ ẩm; GPIO2: bụi |

## Phần mềm và công nghệ

| Nhóm | Công nghệ sử dụng |
| --- | --- |
| Firmware | C++, Arduino framework, FreeRTOS |
| Công cụ build | PlatformIO |
| Giao diện web | HTML, CSS, JavaScript; file được lưu trên SPIFFS |
| Giao tiếp | Wi-Fi, HTTP, WebSocket, JSON, Blynk |
| Web server | ESPAsyncWebServer, AsyncTCP |
| Lưu cấu hình | Preferences / NVS |

## Build nhanh

1. Mở folder `Code_Git` bằng PlatformIO.
2. Copy [network_config.example.h](Code_Git/src/tasks/communication/network_config.example.h) thành `network_config.h` trong cùng folder, rồi điền Wi-Fi và cấu hình Blynk của bạn. File cấu hình thật được Git bỏ qua.
3. Từ thư mục repository, chạy `pio run -d Code_Git`.

Giao diện trong `Code_Git/data/` cần được nạp riêng vào SPIFFS khi triển khai lên board.

## Tài liệu và trạng thái

Xem [cấu trúc source](Code_Git/src/README.md), [hướng dẫn RTOS](docs/rtos-guide.md) và [kết quả kiểm thử, giới hạn hiện tại](docs/validation.md).

Bản RTOS đã build và qua kiểm thử host trước đây; chưa được nghiệm thu trên board thật. Bộ test host đã được gỡ khỏi repository. Các điểm cần kiểm chứng, gồm chính sách cảm biến lỗi và lưu/khôi phục ngưỡng bụi, được ghi trong tài liệu.
