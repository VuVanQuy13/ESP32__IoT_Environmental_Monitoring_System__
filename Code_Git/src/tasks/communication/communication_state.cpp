#include "tasks/communication/communication_state.h"
#include <cmath>

bool RetrySchedule::due(uint32_t now) const {
  return !hasAttempted || static_cast<uint32_t>(now - lastAttemptMs) >= intervalMs;
}
void RetrySchedule::attempted(uint32_t now) {lastAttemptMs = now; hasAttempted = true;}
void RetrySchedule::reset() {hasAttempted = false;}

void CloudOutbox::stage(uint8_t pin, double value, bool force) {
  if (pin >= 13) return;
  auto& slot = slots[pin];
  const bool equal = slot.value == value || (std::isnan(slot.value) && std::isnan(value));
  if (force || !slot.valid || !equal) slot.pending = true;
  slot.value = value;
  slot.valid = true;
}

void CloudOutbox::stageSnapshot(const ControlSnapshot& snapshot) {
  stage(3, snapshot.outputs.temperature);
  stage(4, snapshot.outputs.humidity);
  stage(5, snapshot.outputs.dust);
  stage(6, snapshot.outputs.temperature);
  stage(7, snapshot.outputs.humidity);
  stage(8, snapshot.outputs.dust);
  stage(9, snapshot.mode == ControlMode::Auto ? 1 : 0);
  stage(10, snapshot.thresholds.temperature);
  stage(11, snapshot.thresholds.humidity);
  stage(12, snapshot.thresholds.dust);
}

void CloudOutbox::stageTelemetry(const TelemetryFrame& frame) {
  // Send each new telemetry frame even when numeric readings are unchanged.
  stage(0, frame.blynk.temperature, true);
  stage(1, frame.blynk.humidity, true);
  stage(2, frame.blynk.dust, true);
}

void CloudOutbox::newSession() {
  for (auto& slot : slots) if (slot.valid) slot.pending = true;
  cursor = 0;
  hasWritten = false;
}

bool CloudOutbox::next(uint32_t now, uint8_t& pin, double& value) const {
  if (hasWritten && static_cast<uint32_t>(now - lastWriteMs) < CLOUD_WRITE_INTERVAL_MS) return false;
  for (uint8_t offset = 0; offset < 13; ++offset) {
    const uint8_t index = (cursor + offset) % 13;
    if (slots[index].valid && slots[index].pending) {
      pin = index;
      value = slots[index].value;
      return true;
    }
  }
  return false;
}

void CloudOutbox::sent(uint8_t pin, uint32_t now) {
  if (pin >= 13) return;
  slots[pin].pending = false;
  cursor = (pin + 1) % 13;
  lastWriteMs = now;
  hasWritten = true;
}
