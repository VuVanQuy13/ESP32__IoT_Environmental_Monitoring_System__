#pragma once

#include "shared/app_types.h"

constexpr uint32_t WIFI_RETRY_MS = 30000;
constexpr uint32_t CLOUD_RETRY_MS = 5000;
constexpr uint32_t CLOUD_CONNECT_BUDGET_MS = 750;
constexpr uint32_t CLOUD_WRITE_INTERVAL_MS = 125;

// A deadline expressed as elapsed time, safe across millis() wraparound.
class RetrySchedule {
 public:
  explicit RetrySchedule(uint32_t interval) : intervalMs(interval) {}
  bool due(uint32_t now) const;
  void attempted(uint32_t now);
  void reset();
 private:
  uint32_t intervalMs;
  uint32_t lastAttemptMs = 0;
  bool hasAttempted = false;
};

// CommunicationTask owns this outbox. Coalesce unsent values, pace one pin/write.
class CloudOutbox {
 public:
  void stageSnapshot(const ControlSnapshot& snapshot);
  void stageTelemetry(const TelemetryFrame& frame);
  void newSession();
  bool next(uint32_t now, uint8_t& pin, double& value) const;
  void sent(uint8_t pin, uint32_t now);
 private:
  struct Slot {double value = 0; bool valid = false; bool pending = false;};
  Slot slots[13];
  uint8_t cursor = 0;
  uint32_t lastWriteMs = 0;
  bool hasWritten = false;
  void stage(uint8_t pin, double value, bool force = false);
};
