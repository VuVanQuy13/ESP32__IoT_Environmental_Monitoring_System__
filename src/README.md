# Cấu trúc source theo task

```text
src/
├── main.cpp                     Khởi tạo queue và ba task
├── shared/                      Dữ liệu và tiện ích dùng chung
│   ├── app_types.h              ControlEvent, ControlSnapshot, TelemetryFrame
│   ├── app_queues.h/.cpp        Queue giữa các task
│   └── task_support.h          Kiểm tra tài nguyên và báo lỗi
└── tasks/
    ├── sensor/
    │   ├── sensor_task.h/.cpp   Task đọc cảm biến
    │   └── SharpGP2Y10.h/.cpp   Driver cảm biến bụi
    ├── control/
    │   ├── control_task.h/.cpp  Task ghi GPIO và lưu Preferences
    │   ├── control_state.h/.cpp Xử lý sự kiện và chuyển trạng thái
    │   └── control_logic.h/.cpp Quy tắc Auto/Manual
    └── communication/
        ├── communication_task.h/.cpp  Task giao tiếp
        ├── communication_state.h/.cpp Lịch thử lại và dữ liệu chờ gửi
        ├── network_connection.h/.cpp  Wi-Fi, Blynk và callback Blynk
        ├── network_config.h           Cấu hình kết nối hiện có
        ├── async_dns.h/.cpp           Phân giải DNS bất đồng bộ
        ├── bounded_wifi_client.h/.cpp Transport riêng cho Blynk
        ├── web_dashboard.h/.cpp       HTTP/WebSocket và phát JSON
        └── web_command.h/.cpp         Phân tích lệnh từ web
```

Ký hiệu `.h/.cpp` trong cây đại diện cho hai file cùng tên. Mỗi folder task chứa task và các module phục vụ trách nhiệm đó; không phải mỗi file là một task mới. `shared` không tạo thêm task.

Các include nội bộ ghi đường dẫn từ `src`, ví dụ `#include "shared/app_queues.h"` hoặc `#include "tasks/control/control_state.h"`. PlatformIO build các file `.cpp` trong cây `src`.

Đọc theo thứ tự: `main.cpp` → `shared/app_types.h` → `shared/app_queues.cpp` → `tasks/sensor/sensor_task.cpp` → `tasks/control/control_task.cpp` → `tasks/communication/communication_task.cpp`.

Giao diện trình duyệt vẫn ở `Code_Git/data/`. Hướng dẫn RTOS có link tới từng dòng đã được cập nhật tại [reading_guide.md](../../docs/rtos-guide.md). Việc chia folder không đổi số task, priority, core, queue hoặc quy tắc điều khiển.
