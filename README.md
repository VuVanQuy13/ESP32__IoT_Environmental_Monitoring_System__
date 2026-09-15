# ESP32 Environmental Monitor — FreeRTOS

Hệ thống giám sát nhiệt độ, độ ẩm và bụi bằng ESP32, DHT11 và Sharp GP2Y10; điều khiển ba ngõ ra và cung cấp dashboard web/Blynk.

## Chức năng

- Đọc cảm biến, xem số đo và chỉnh ngưỡng trên web/Blynk.
- **Auto:** điều khiển GPIO theo ngưỡng. **Manual:** điều khiển từng ngõ ra qua Blynk; thao tác nút ngõ ra tự chuyển toàn hệ thống sang Manual.
- Lưu ngưỡng bằng Preferences/NVS. Khởi động ở Auto; không lưu mode/ngõ ra qua reboot.
- Ba task FreeRTOS: Sensor, Control, Communication, trao đổi bản sao dữ liệu qua queue.
- Sensor/Control được tạo trước các lần thử kết nối mạng; Wi-Fi/Blynk có lịch kết nối lại.

## Phần cứng và điều khiển

| Thành phần | Chân ESP32 | Chức năng |
| --- | --- | --- |
| ESP32 DOIT DevKit V1 | — | Board cấu hình trong PlatformIO |
| DHT11 | GPIO13 | Nhiệt độ và độ ẩm |
| Sharp GP2Y10 | GPIO34 / GPIO25 | ADC / điều khiển LED cảm biến |
| Ngõ ra nhiệt độ | GPIO22 | HIGH khi nhiệt độ > ngưỡng trong Auto |
| Ngõ ra độ ẩm | GPIO5 | HIGH khi độ ẩm < ngưỡng trong Auto |
| Ngõ ra bụi | GPIO2 | HIGH khi bụi > ngưỡng trong Auto |

Ngưỡng mặc định khi chưa lưu NVS: **35 °C**, **80%**, **100 cho bụi**. Khi số đo bằng ngưỡng, điều kiện bật tương ứng là sai. Không có hysteresis. Bảng mô tả mức GPIO; cách bật tải phụ thuộc mạch nối ngoài.

**Đơn vị bụi chưa thống nhất:** chú thích driver ghi mg/m³, web ghi µg/m³, nhưng dữ liệu truyền thẳng không đổi đơn vị. Công thức hiện tại là `max((raw * (3.3 / 4096) - 0.6) / 0.5, 0)`. Cần xác minh đơn vị và hiệu chuẩn trước khi diễn giải số đo/ngưỡng bụi.

## Cấu trúc và công nghệ

```text
.
├── platformio.ini       Board, framework, thư viện
├── src/                 Firmware C++: main, shared, các task
├── data/                index.html, Style.css, main.js cho SPIFFS
└── docs/                Hướng dẫn RTOS và kết quả kiểm thử lịch sử
```

Stack: Arduino framework, FreeRTOS, Wi-Fi, ESPAsyncWebServer/AsyncTCP, Arduino_JSON, Blynk, Preferences và SPIFFS. Dependency trực tiếp nằm trong [platformio.ini](platformio.ini). `espressif32` chưa khóa phiên bản; kết quả build lịch sử không bảo đảm cho mọi phiên bản framework tải mới.

## Cấu hình và build

Mở **thư mục gốc chứa `platformio.ini`** bằng PlatformIO IDE hoặc dùng PlatformIO Core CLI. Chạy các lệnh dưới đây tại thư mục đó.

1. Nếu chưa có `src/tasks/communication/network_config.h`, sao chép [network_config.example.h](src/tasks/communication/network_config.example.h) thành file này trong cùng thư mục.
2. Điền `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, `BLYNK_AUTH_TOKEN`, `networkConfig::ssid` và `networkConfig::password` của bạn.
3. Build firmware:

   ```sh
   pio run -e esp32doit-devkit-v1
   ```

**Cấu hình riêng:** trong trạng thái repo được rà soát, `network_config.h` đang được Git theo dõi và `.gitignore` chưa loại trừ file này. Không đưa token/mật khẩu cá nhân vào commit hoặc bản chia sẻ.

### Nạp board và SPIFFS

Kết nối board bằng USB, chạy lần lượt:

```sh
pio run -e esp32doit-devkit-v1 -t upload
pio run -e esp32doit-devkit-v1 -t uploadfs
pio device monitor -b 9600
```

`upload` nạp firmware; `uploadfs` nạp giao diện trong `data/` vào SPIFFS. Khi sửa HTML/CSS/JS, nạp lại filesystem. Nếu có nhiều cổng nối tiếp, thêm `--upload-port COMx` vào lệnh nạp và `-p COMx` vào monitor, thay `COMx` bằng cổng thực tế.

## Sử dụng

### Dashboard web

1. Cho ESP32 kết nối Wi-Fi đã cấu hình; tìm IP trong danh sách DHCP của router. Firmware hiện log trạng thái kết nối, không in IP.
2. Mở `http://<IP-ESP32>/` từ máy có thể truy cập ESP32 trong LAN.
3. Xem số đo và chỉnh slider: nhiệt độ 0–100, độ ẩm 0–100, bụi 0–250; bước chỉnh 1.

Web dùng HTTP cổng 80 và WebSocket `/ws`; giao diện chỉ có số đo/ngưỡng. Nút mode và Manual nằm trên Blynk. Bootstrap/Font Awesome tải từ CDN nên trình bày có thể thiếu khi trình duyệt không có Internet. HTTP/WebSocket hiện không có xác thực hay TLS.

### Blynk virtual pins

Thiết lập datastream/widget tương ứng với các pin firmware sử dụng:

| Pin | Nội dung | Chiều dữ liệu |
| --- | --- | --- |
| V0 / V1 / V2 | Nhiệt độ / độ ẩm / bụi | ESP32 → Blynk |
| V3 / V4 / V5 | Trạng thái ngõ ra tương ứng, 0 hoặc 1 | ESP32 → Blynk |
| V6 / V7 / V8 | Nút ngõ ra tương ứng, 1 bật / 0 tắt | Hai chiều |
| V9 | 1 Auto / 0 Manual | Hai chiều |
| V10 / V11 / V12 | Ngưỡng nguyên nhiệt độ / độ ẩm / bụi | Hai chiều |

Chọn Auto hoặc đổi ngưỡng có hiệu lực điều khiển ở mẫu tiếp theo. Khi cloud kết nối lại, ESP32 gửi trạng thái cục bộ hiện tại; không gọi `syncAll()` lấy trạng thái cloud cũ.

## Vận hành và giới hạn

- Telemetry được tạo mỗi khoảng 2 giây theo timestamp mẫu. Sensor nghỉ 10 ms sau mỗi lượt đọc; đây không phải chu kỳ lấy mẫu chính xác 10 ms.
- Wi-Fi thử lại theo lịch 30 giây; Blynk theo lịch 5 giây sau lần thử. Trong một phiên cloud, các lần gửi virtual pin thành công cách nhau ít nhất 125 ms.
- Mailbox chỉ giữ dữ liệu mới nhất, không lưu lịch sử khi mất mạng. Mất Wi-Fi khiến web LAN không truy cập được; Sensor/Control có luồng xử lý riêng.
- **Ngưỡng bụi sau reboot:** web lưu khóa `DoBui`, khởi động và Blynk dùng `Bui`. Ngưỡng bụi đặt trên web có thể không được khôi phục.
- **Cảm biến lỗi:** ở mẫu đến lịch telemetry, nếu nhiệt độ/độ ẩm NaN hoặc bụi ngoài 0–1000, Blynk nhận `{50,50,50}`; Auto cũng dùng bộ này ở lần đó. Web nhận mẫu thô. Đây là hành vi hiện có, chưa phải chính sách an toàn đã nghiệm thu.

## Tài liệu và kiểm chứng

- [Cấu trúc source và luồng dữ liệu](src/README.md).
- [Hướng dẫn đọc RTOS](docs/rtos-guide.md).
- [Kết quả kiểm thử lịch sử](docs/validation.md).

Bản RTOS từng build và qua kiểm thử host theo ghi nhận trong tài liệu. Bộ test host/fixture đã được gỡ khỏi repository; chưa nghiệm thu bản RTOS trên board thật. Các lệnh nạp trên đây là hướng dẫn triển khai, không phải xác nhận đã chạy trên thiết bị.
