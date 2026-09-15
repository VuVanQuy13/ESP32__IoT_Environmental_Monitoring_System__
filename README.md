# 🌍 IoT Environmental Monitoring System with ESP32 + WebSocket + Blynk

## 📌 Introduction
This is an **IoT environmental monitoring system** built using the **ESP32**.
The system measures **temperature, humidity, and dust concentration**, then displays the data in real time on:

- A **Web Dashboard** (HTML/CSS + WebSocket) hosted directly on the ESP32.
- The **Blynk IoT app** for remote monitoring and control over the Internet.

The system supports **Automatic/Manual modes** and allows alert thresholds to be adjusted directly through the **web interface** or the **Blynk app**. Threshold values are stored using **Preferences (ESP32 emulated EEPROM)** so they are retained after a restart.

---

## ⚙️ Features

- 🌡 Measure **temperature & humidity** using a **DHT11** sensor.
- 🌫 Measure **dust concentration** using a **Sharp GP2Y10** sensor with a custom C++ driver.
- 📡 Display live readings on a **Web Dashboard** using WebSocket, SPIFFS, and Bootstrap.
- 📱 Connect to **Blynk IoT Cloud** for remote monitoring and control.
- 🔧 Adjust **control thresholds** through web sliders or the Blynk app.
- 💾 Store thresholds using **Preferences / NVS**. The current dust threshold storage keys differ between web updates and startup, so web dust settings may not be restored after a restart.
- 💡 Support **Automatic/Manual modes** for three GPIO outputs; Blynk output commands automatically select Manual mode.
- 🧵 Organize firmware into **three FreeRTOS tasks**:
  - **SensorTask:** read DHT11 and Sharp GP2Y10 sensors.
  - **ControlTask:** process commands, apply Auto/Manual logic, update GPIO outputs, and save thresholds.
  - **Communication:** service Wi-Fi/Blynk, publish web data, and schedule reconnection attempts.
- 📬 Exchange sensor samples and commands through a **FIFO queue**, with single-slot queues retaining the latest control snapshot and telemetry.
- 🛡 Monitor each application task using the **Task Watchdog Timer**.
- 🔄 Start sensor acquisition and control before network connection attempts; retry Wi-Fi/Blynk and resend the current local state when Blynk reconnects.

---

## 🛠 Technologies & Tools

- **Firmware Language:** C++ with the Arduino framework.
- **Microcontroller:** ESP32 DOIT DevKit V1.
- **RTOS:** FreeRTOS tasks, priorities, task pinning, queues, and task delays.
- **IoT Platform:** Blynk IoT Cloud.
- **Web:** HTML, CSS, JavaScript, Bootstrap, Font Awesome, HTTP, and WebSocket.
- **Sensors:** DHT11 and Sharp GP2Y10 with a custom C++ driver.
- **Networking Libraries:** WiFi, ESPAsyncWebServer, AsyncTCP, Blynk, and lwIP for asynchronous DNS and socket operations.
- **Data & Storage:** Arduino_JSON, SPIFFS for web assets, and Preferences / NVS for thresholds.
- **Monitoring:** ESP32 Task Watchdog (`esp_task_wdt`) and structured Serial logs.
- **Development Tools:** PlatformIO with the Arduino framework, Git, and GitHub.

---

## 📂 Project Structure

```text
.
├── platformio.ini                 # Board, framework, and library dependencies
├── src/
│   ├── main.cpp                   # Initialize watchdog, queues, and application tasks
│   ├── README.md                  # Firmware architecture and data flow
│   ├── shared/
│   │   ├── app_types.h            # Events, sensor readings, snapshots, and telemetry
│   │   ├── app_queues.h/.cpp       # FreeRTOS queue interfaces and implementation
│   │   └── task_support.h         # Resource checks and fatal-error logging
│   └── tasks/
│       ├── sensor/
│       │   ├── sensor_task.h/.cpp          # Sensor acquisition task
│       │   └── SharpGP2Y10.h/.cpp          # Custom dust sensor driver
│       ├── control/
│       │   ├── control_task.h/.cpp         # GPIO and Preferences owner
│       │   ├── control_logic.h/.cpp        # Auto/Manual decisions
│       │   └── control_state.h/.cpp        # Event processing and state transitions
│       └── communication/
│           ├── communication_task.h/.cpp  # Network and telemetry task
│           ├── communication_state.h/.cpp # Retry schedules and Blynk outbox
│           ├── network_connection.h/.cpp  # Wi-Fi, Blynk, and command callbacks
│           ├── network_config.example.h   # Wi-Fi/Blynk configuration template
│           ├── network_config.h           # Device-specific configuration
│           ├── async_dns.h/.cpp           # Asynchronous DNS resolver
│           ├── bounded_wifi_client.h/.cpp # Blynk socket transport
│           ├── web_dashboard.h/.cpp       # HTTP/WebSocket server and JSON publishing
│           └── web_command.h/.cpp         # Web command parser
├── data/
│   ├── index.html                 # Dashboard markup
│   ├── Style.css                  # Dashboard styling
│   └── main.js                    # WebSocket client and threshold sliders
├── docs/
│   ├── rtos-guide.md              # RTOS source walkthrough
│   ├── validation.md              # Historical validation results and limitations
│   ├── codebase-map.md            # Module and dependency map
│   └── context.md                 # Project context and decisions
└── README.md
```

The `.h/.cpp` notation represents two files with the same base name. Web assets in `data/` are uploaded separately to SPIFFS.

---

## Web Dashboard Interface:
<img width="1917" height="913" alt="image" src="https://github.com/user-attachments/assets/ed7c83a6-ef63-4d88-a2bc-2574b901cb92" />

---

## BLYNK Interface:
<img width="1617" height="742" alt="image" src="https://github.com/user-attachments/assets/f35b83a9-2b18-497a-9ec4-69e8e346aa2f" />

---

## Altium:

- **SCH**:
<img width="1081" height="717" alt="image" src="https://github.com/user-attachments/assets/3a977b6c-a00d-4419-afe5-d5c3162974ae" />

- **PCB**:
<img width="1119" height="712" alt="image" src="https://github.com/user-attachments/assets/37066228-31f4-4d04-9528-03fb1f0f843e" />

---

## Completed Circuit Board:

![ALtium](https://github.com/user-attachments/assets/f8a46e01-963e-4a49-8875-94911fc64c47)

![Bottom View](https://github.com/user-attachments/assets/4c70fc32-14a1-4549-aa13-a8ec93be3b7f)
