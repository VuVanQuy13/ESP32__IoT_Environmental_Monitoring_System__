#include "shared/app_queues.h"
#include <type_traits>

namespace {
QueueHandle_t controlQueue = nullptr;
QueueHandle_t snapshotQueue = nullptr;
QueueHandle_t telemetryQueue = nullptr;
}

static_assert(std::is_trivially_copyable<ControlEvent>::value, "Queue copies event bytes");
static_assert(std::is_trivially_copyable<ControlSnapshot>::value, "Queue copies snapshot bytes");
static_assert(std::is_trivially_copyable<TelemetryFrame>::value, "Queue copies telemetry bytes");

bool createAppQueues() {
  if (controlQueue != nullptr) return false;
  controlQueue = xQueueCreate(CONTROL_QUEUE_LENGTH, sizeof(ControlEvent));
  snapshotQueue = xQueueCreate(1, sizeof(ControlSnapshot));
  telemetryQueue = xQueueCreate(1, sizeof(TelemetryFrame));
  if (controlQueue && snapshotQueue && telemetryQueue) return true;
  if (controlQueue) vQueueDelete(controlQueue);
  if (snapshotQueue) vQueueDelete(snapshotQueue);
  if (telemetryQueue) vQueueDelete(telemetryQueue);
  controlQueue = snapshotQueue = telemetryQueue = nullptr;
  return false;
}

bool postControlEvent(const ControlEvent& event, TickType_t waitTicks) {
  return controlQueue && xQueueSend(controlQueue, &event, waitTicks) == pdPASS;
}

bool receiveControlEvent(ControlEvent& event, TickType_t waitTicks) {
  return controlQueue && xQueueReceive(controlQueue, &event, waitTicks) == pdPASS;
}

bool publishControlSnapshot(const ControlSnapshot& snapshot) {
  return snapshotQueue && xQueueOverwrite(snapshotQueue, &snapshot) == pdPASS;
}

bool peekControlSnapshot(ControlSnapshot& snapshot) {
  return snapshotQueue && xQueuePeek(snapshotQueue, &snapshot, 0) == pdPASS;
}

bool publishTelemetryFrame(const TelemetryFrame& frame) {
  return telemetryQueue && xQueueOverwrite(telemetryQueue, &frame) == pdPASS;
}

bool takeTelemetryFrame(TelemetryFrame& frame) {
  return telemetryQueue && xQueueReceive(telemetryQueue, &frame, 0) == pdPASS;
}
