#pragma once

#include "shared/app_types.h"

constexpr uint8_t TEMPERATURE_OUTPUT = 1;
constexpr uint8_t HUMIDITY_OUTPUT = 2;
constexpr uint8_t DUST_OUTPUT = 4;
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;

struct ControlState {
  ControlSnapshot snapshot;
  uint32_t lastTelemetryMs = 0;
};

struct ControlEffects {
  bool accepted = false;
  uint8_t writeMask = 0;
  bool persistThreshold = false;
  bool publishTelemetry = false;
  TelemetryFrame telemetry{};
};

// ControlTask is the only runtime caller that changes ControlState.
ControlEffects processControlEvent(ControlState& state, const ControlEvent& event);
const char* thresholdStorageKey(OutputChannel channel, CommandSource source);
