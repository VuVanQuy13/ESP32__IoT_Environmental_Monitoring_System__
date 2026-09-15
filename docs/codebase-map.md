# Codebase map

| Path | Responsibility |
| --- | --- |
| README.md | Concise project overview: features, hardware, software stack, quick build and status |
| src/main.cpp | Watchdog, queue creation, start Control/Sensor/Communication; idle Arduino loop |
| src/shared/ | Plain message types, FreeRTOS queues, resource error helper |
| src/tasks/sensor/ | Sensor task, DHT integration and Sharp driver |
| src/tasks/control/ | Pure control logic, event reducer, GPIO and Preferences effects |
| src/tasks/communication/ | Task scheduling, outbox/retry, Wi-Fi/Blynk, async DNS/socket adapter, web/parser |
| data/ | Browser HTML/CSS/JS served from SPIFFS |
| docs/rtos-guide.md | Detailed reading guide with source links |
| docs/validation.md | Recorded check results and hardware limits |

Flow: sensor samples and web/Blynk commands -> control FIFO -> ControlTask -> GPIO/Preferences -> snapshot/telemetry mailboxes -> Communication -> clients.

Control owns state; other tasks receive copies. Web callbacks run in library task context. Blynk calls remain with Communication. Network operations are isolated from control decisions; no hard deadline promised. Existing sensor fallback and dust NVS key discrepancy remain documented.

Includes are relative to src. PlatformIO build: pio run -e esp32doit-devkit-v1. Copy network_config.example.h to network_config.h (currently tracked; do not commit personal credentials) and configure locally first. Host tests and fixtures were deleted at the user's explicit request; docs/validation.md records historical results only.

Cleanup retains dependency cache locally through ignore rules; removed historical migration/log folders, generated build products and host tests. Firmware build instructions remain in README. RTOS board validation is the next separately approved step.

README maintenance: root README covers setup/upload/use and src/README.md covers module ownership, queues and web protocol. For pin edits use sensor_task.cpp/control_task.cpp; for control rules use control_logic.cpp/control_state.cpp; coordinate web protocol edits with data/main.js. Sharp driver comments say mg/m3 while HTML labels ug/m3 without conversion. Current .gitignore does not exclude network_config.h. Validation is historical; README update was documentation-only.
