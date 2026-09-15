#include "tasks/control/control_task.h"
#include "tasks/control/control_state.h"
#include "shared/app_queues.h"
#include "shared/task_support.h"
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <freertos/task.h>

namespace {
constexpr int TEMPERATURE_PIN = 22;
constexpr int HUMIDITY_PIN = 5;
constexpr int DUST_PIN = 2;
constexpr uint32_t CONTROL_STACK_BYTES = 4096;
TaskHandle_t controlHandle = nullptr;

void writeOutputs(uint8_t mask, const OutputStates& outputs) {
  if (mask & TEMPERATURE_OUTPUT) digitalWrite(TEMPERATURE_PIN, outputs.temperature ? HIGH : LOW);
  if (mask & HUMIDITY_OUTPUT) digitalWrite(HUMIDITY_PIN, outputs.humidity ? HIGH : LOW);
  if (mask & DUST_OUTPUT) digitalWrite(DUST_PIN, outputs.dust ? HIGH : LOW);
}

void controlTask(void*) {
  // No other task or callback holds references to state or Preferences.
  ControlState state;
  Preferences prefs;
  requireTaskResource(esp_task_wdt_add(nullptr) == ESP_OK, "control_wdt_add");
  pinMode(TEMPERATURE_PIN, OUTPUT);
  pinMode(HUMIDITY_PIN, OUTPUT);
  pinMode(DUST_PIN, OUTPUT);
  state.snapshot.outputs = {digitalRead(TEMPERATURE_PIN) == HIGH,
                            digitalRead(HUMIDITY_PIN) == HIGH,
                            digitalRead(DUST_PIN) == HIGH};
  requireTaskResource(prefs.begin("Gia tri nguong", false), "preferences_begin");
  state.snapshot.thresholds = {prefs.getInt("NhietDo", 35), prefs.getInt("DoAm", 80),
                               prefs.getInt("Bui", 100)};
  state.snapshot.thresholdRevision = 1;
  requireTaskResource(publishControlSnapshot(state.snapshot), "initial_snapshot");
  Serial.printf("event=task_started task=ControlTask core=%d stack_bytes=%u\n",
                xPortGetCoreID(), CONTROL_STACK_BYTES);

  for (;;) {
    ControlEvent event;
    // Sleeping on the queue lets other tasks run; timeout allows our own WDT check-in.
    if (receiveControlEvent(event, pdMS_TO_TICKS(100))) {
      const auto effects = processControlEvent(state, event);
      if (!effects.accepted) {
        Serial.printf("event=command_rejected reason=invalid type=%d\n", static_cast<int>(event.type));
      } else {
        writeOutputs(effects.writeMask, state.snapshot.outputs);
        if (effects.persistThreshold) {
          const char* key = thresholdStorageKey(event.channel, event.source);
          requireTaskResource(key != nullptr, "threshold_key");
          const bool saved = prefs.putInt(key, event.value) == sizeof(int32_t);
          Serial.printf("event=threshold_applied channel=%d value=%d saved=%d\n",
                        static_cast<int>(event.channel), event.value, saved);
          // As before, RAM threshold remains applied if persistence fails; report it.
          if (!saved) Serial.println("event=storage_error operation=putInt");
        }
        if (event.type == EventType::SetMode || event.type == EventType::ManualOutput) {
          Serial.printf("event=control_command type=%d mode=%s write_mask=%u\n",
                        static_cast<int>(event.type),
                        state.snapshot.mode == ControlMode::Auto ? "auto" : "manual",
                        effects.writeMask);
        }
        if (effects.publishTelemetry) {
          requireTaskResource(publishTelemetryFrame(effects.telemetry), "telemetry_publish");
        }
        requireTaskResource(publishControlSnapshot(state.snapshot), "snapshot_publish");
      }
    }
    requireTaskResource(esp_task_wdt_reset() == ESP_OK, "control_wdt_reset");
    // Bound CPU use even when a producer continuously fills the input queue.
    vTaskDelay(1);
  }
}
}

bool startControlTask() {
  if (controlHandle != nullptr) return false;
  return xTaskCreatePinnedToCore(controlTask, "ControlTask", CONTROL_STACK_BYTES, nullptr,
                                 2, &controlHandle, ARDUINO_RUNNING_CORE) == pdPASS;
}
