# Kết quả kiểm thử đã thực hiện

Kết quả gần nhất sau khi chia folder task:

| Kiểm tra | Kết quả |
| --- | --- |
| PlatformIO build ESP32 DOIT DevKit V1 | PASS; static RAM 46.364 byte, app Flash 919.509 byte |
| Logic điều khiển | 50 trường hợp tại O0/O2 PASS |
| State, parser, queue, SensorTask/ControlTask | O0/O2 PASS |
| CommunicationTask, DNS, transport, startup | O0/O2 PASS |
| So sánh với fixture logic cũ | 20.736 trường hợp khớp GPIO/mode/telemetry cuối |
| Di chuyển folder | 30 file production giữ nội dung ngoài thay đổi đường dẫn include |

Các test chạy mã ứng dụng với RTOS/network/hardware doubles trên PC. Chúng không mô phỏng đầy đủ scheduler, radio hoặc board thật, không chứng minh độ trễ vật lý hay stack còn dư lúc chạy. Static RAM trong báo cáo build không bao gồm toàn bộ cấp phát runtime.

Các log build, bản hash trung gian và script chuyển đổi dùng một lần đã được dọn khi chuẩn bị repository. Theo yêu cầu người dùng, bộ test host và fixture cũng đã được xóa. Các kết quả trên là ghi nhận lịch sử, không phải một bộ kiểm thử còn có thể chạy lại từ repository hiện tại. Hướng dẫn build firmware vẫn có trong [README](../README.md).

Chưa thực hiện nghiệm thu bản RTOS trên board: boot offline, Wi-Fi/DNS/cloud mất và phục hồi, web/Blynk thao tác đồng thời, Auto/Manual, ngưỡng sau reboot, watchdog và bộ nhớ khi chạy lâu. Phần đo/kiểm thử này là bước tiếp theo, chờ người dùng duyệt.
