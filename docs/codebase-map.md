# Codebase map

| Path | Responsibility |
| --- | --- |
| README.md | Concise project overview: features, hardware, software stack, quick build and status |
| Code_Git/src/main.cpp | Watchdog, queue creation, start Control/Sensor/Communication; idle Arduino loop |
| Code_Git/src/shared/ | Plain message types, FreeRTOS queues, resource error helper |
| Code_Git/src/tasks/sensor/ | Sensor task, DHT integration and Sharp driver |
| Code_Git/src/tasks/control/ | Pure control logic, event reducer, GPIO and Preferences effects |
| Code_Git/src/tasks/communication/ | Task scheduling, outbox/retry, Wi-Fi/Blynk, async DNS/socket adapter, web/parser |
| Code_Git/data/ | Browser HTML/CSS/JS served from SPIFFS |
| docs/rtos-guide.md | Detailed reading guide with source links |
| docs/validation.md | Recorded check results and hardware limits |

Flow: sensor samples and web/Blynk commands -> control FIFO -> ControlTask -> GPIO/Preferences -> snapshot/telemetry mailboxes -> Communication -> clients.

Control owns state; other tasks receive copies. Web callbacks run in library task context. Blynk calls remain with Communication. Network operations are isolated from control decisions; no hard deadline promised. Existing sensor fallback and dust NVS key discrepancy remain documented.

Includes are relative to src. PlatformIO build: pio run -d Code_Git. Copy network_config.example.h to ignored network_config.h and configure locally first. Host tests and fixtures were deleted at the user's explicit request; docs/validation.md records historical results only.

Cleanup retains dependency cache locally through ignore rules; removed historical migration/log folders, generated build products and host tests. Firmware build instructions remain in README. RTOS board validation is the next separately approved step.
