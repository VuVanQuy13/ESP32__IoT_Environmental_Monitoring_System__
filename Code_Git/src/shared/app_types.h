#pragma once

#include <stdint.h>

// Plain values: no Arduino objects, pointers or RTOS dependencies.
struct SensorReadings {
  float temperature;
  float humidity;
  float dust;
};

struct Thresholds {
  int temperature;
  int humidity;
  int dust;
};

enum class ControlMode { Auto, Manual };
enum class OutputChannel { Temperature, Humidity, Dust };

struct OutputStates {
  bool temperature;
  bool humidity;
  bool dust;
};

struct AutoDecision {
  bool shouldWrite;
  OutputStates outputs;
};

struct ManualCommand {
  OutputChannel channel;
  int value;  // Preserve Blynk semantics: exactly 1 is ON, anything else OFF.
};

struct ManualDecision {
  bool accepted;
  ControlMode mode;
  OutputChannel channel;
  bool on;
};

enum class EventType { Sample, ManualOutput, SetMode, SetThreshold, GetThresholds };
enum class CommandSource { Web, Blynk };

// Queue copies the whole value. No pointers into a callback's temporary buffer.
struct ControlEvent {
  EventType type = EventType::GetThresholds;
  CommandSource source = CommandSource::Blynk;
  OutputChannel channel = OutputChannel::Temperature;
  int value = 0;
  SensorReadings readings{-999, -999, -999};
  uint32_t sampledAtMs = 0;
};

struct ControlSnapshot {
  ControlMode mode = ControlMode::Auto;
  Thresholds thresholds{35, 80, 100};
  SensorReadings readings{-999, -999, -999};
  OutputStates outputs{false, false, false};
  uint32_t outputRevision = 0;
  uint32_t thresholdRevision = 0;
};

struct TelemetryFrame {
  SensorReadings web;
  SensorReadings blynk;
  uint32_t sampledAtMs;
};
