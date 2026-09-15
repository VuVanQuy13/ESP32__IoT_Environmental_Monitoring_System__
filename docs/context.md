# Project context

- Root README rewritten at user request as a concise Vietnamese introduction: features, hardware/pins, software stack, quick build and validation status. Firmware unchanged.
- ESP32 DOIT DevKit V1 environmental controller: DHT11, Sharp GP2Y10, GPIO22/5/2, Auto/Manual, Preferences, SPIFFS web and Blynk.
- User confirmed the original firmware ran on hardware. RTOS implementation through step 4 is complete and host-tested; actual RTOS board acceptance is pending.
- Work one approved step at a time. Step 5 (runtime measurements and board validation) has NOT been authorized. Do not upload or push without authorization.
- Source organized into tasks/sensor, tasks/control, tasks/communication and shared. Control exclusively owns application state, Preferences and three control outputs; Sensor owns drivers; Communication owns Blynk runtime. AsyncTCP callbacks enqueue commands.
- Three tasks on Arduino core 1: Sensor priority1/stack4096, Control priority2/stack4096, Communication priority1/stack8192. Each registers/resets its watchdog. main creates queues and tasks before network connection attempts.
- Control FIFO16; snapshot and telemetry mailboxes each1. Callbacks reject full queues; Sensor waits up to100ms then logs a dropped sample. Mailboxes keep latest values, not history.
- Preserve existing control logic, sensor timing, fallback policy and NVS keys. Known dust key difference: web DoBui vs boot/Blynk Bui. Do not claim this was fixed.
- Communication retries Wi-Fi30s/cloud5s, paces cloud pin writes125ms, resends local state on reconnect without syncAll. Async DNS uses persistent callback context and a result queue. Individual socket timeouts do not establish a hard real-time guarantee.
- Latest pre-cleanup checks: ESP32 build PASS RAM46364/Flash919509; all host suites O0/O2 PASS; 20736 baseline comparisons PASS. See docs/validation.md. No hardware acceptance claimed.
- User authorized removing redundant files for GitHub. Removed generated build/host-test outputs, historical logs/hash dumps, one-time migration scripts, obsolete step3 guide and boilerplate READMEs. Dependency cache remains locally ignored.
- Consolidated source-linked guide to docs/rtos-guide.md with repository-relative links. User subsequently explicitly requested deleting the host test folder and updating README. Removed tests/fixtures and obsolete run instructions; retain historical validation results clearly labeled. Firmware source/data/config unchanged; no new test or board-validation claim.
- network_config.h remains local and ignored; network_config.example.h documents placeholders. Never print credential values. Preparing the Git index must replace stale source snapshots that previously contained credentials. No commit or push authorized by this cleanup request.
