#include "tasks/control/control_logic.h"

AutoDecision evaluateAuto(ControlMode mode, const SensorReadings& readings,
                          const Thresholds& thresholds) {
  // Preserve the original sentinel/NaN behavior; this is not a new validity policy.
  if (mode != ControlMode::Auto ||
      (readings.temperature == -999 && readings.humidity == -999 &&
       readings.dust == -999)) {
    return {false, {false, false, false}};
  }
  return {true, {readings.temperature > thresholds.temperature,
                 readings.humidity < thresholds.humidity,
                 readings.dust > thresholds.dust}};
}

ManualDecision evaluateManual(const ManualCommand& command) {
  switch (command.channel) {
    case OutputChannel::Temperature:
    case OutputChannel::Humidity:
    case OutputChannel::Dust:
      return {true, ControlMode::Manual, command.channel, command.value == 1};
  }
  return {false, ControlMode::Manual, command.channel, false};
}

ControlMode decodeModeCommand(int value) {
  return value == 1 ? ControlMode::Auto : ControlMode::Manual;
}
