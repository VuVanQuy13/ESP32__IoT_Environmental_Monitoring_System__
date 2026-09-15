#include "tasks/control/control_state.h"
#include "tasks/control/control_logic.h"
#include <cmath>

namespace {
bool validChannel(OutputChannel channel) {
  return channel == OutputChannel::Temperature || channel == OutputChannel::Humidity ||
         channel == OutputChannel::Dust;
}

SensorReadings blynkReadings(SensorReadings readings) {
  // Preserve the original sendSensorBlynk fallback, including its NaN semantics.
  if (std::isnan(readings.temperature) || std::isnan(readings.humidity) ||
      readings.dust < 0 || readings.dust > 1000) {
    return {50, 50, 50};
  }
  return readings;
}
}

ControlEffects processControlEvent(ControlState& state, const ControlEvent& event) {
  ControlEffects effects;
  auto& snapshot = state.snapshot;
  const auto previousMode = snapshot.mode;
  const auto previousOutputs = snapshot.outputs;
  switch (event.type) {
    case EventType::Sample: {
      snapshot.readings = event.readings;
      if (static_cast<uint32_t>(event.sampledAtMs - state.lastTelemetryMs) >=
          TELEMETRY_INTERVAL_MS) {
        state.lastTelemetryMs = event.sampledAtMs;
        effects.publishTelemetry = true;
        effects.telemetry = {event.readings, blynkReadings(event.readings), event.sampledAtMs};
        // Previously: send web -> Blynk fallback -> Auto. Network now gets copies.
        snapshot.readings = effects.telemetry.blynk;
      }
      const auto decision = evaluateAuto(snapshot.mode, snapshot.readings, snapshot.thresholds);
      if (decision.shouldWrite) {
        snapshot.outputs = decision.outputs;
        effects.writeMask = TEMPERATURE_OUTPUT | HUMIDITY_OUTPUT | DUST_OUTPUT;
      }
      break;
    }
    case EventType::ManualOutput: {
      const auto decision = evaluateManual({event.channel, event.value});
      if (!decision.accepted) return effects;
      snapshot.mode = decision.mode;
      switch (decision.channel) {
        case OutputChannel::Temperature:
          snapshot.outputs.temperature = decision.on;
          effects.writeMask = TEMPERATURE_OUTPUT;
          break;
        case OutputChannel::Humidity:
          snapshot.outputs.humidity = decision.on;
          effects.writeMask = HUMIDITY_OUTPUT;
          break;
        case OutputChannel::Dust:
          snapshot.outputs.dust = decision.on;
          effects.writeMask = DUST_OUTPUT;
          break;
      }
      break;
    }
    case EventType::SetMode:
      snapshot.mode = decodeModeCommand(event.value);
      break;  // As before, Auto applies when the next sample is processed.
    case EventType::SetThreshold:
      if (!validChannel(event.channel) ||
          (event.source != CommandSource::Web && event.source != CommandSource::Blynk)) {
        return effects;
      }
      switch (event.channel) {
        case OutputChannel::Temperature: snapshot.thresholds.temperature = event.value; break;
        case OutputChannel::Humidity: snapshot.thresholds.humidity = event.value; break;
        case OutputChannel::Dust: snapshot.thresholds.dust = event.value; break;
      }
      effects.persistThreshold = true;
      ++snapshot.thresholdRevision;
      break;
    case EventType::GetThresholds:
      ++snapshot.thresholdRevision;
      break;
    default:
      return effects;
  }
  const bool outputsChanged = previousOutputs.temperature != snapshot.outputs.temperature ||
                              previousOutputs.humidity != snapshot.outputs.humidity ||
                              previousOutputs.dust != snapshot.outputs.dust;
  // GPIO still follows each Auto decision, but network publication is state-based.
  if (previousMode != snapshot.mode ||
      (effects.writeMask != 0 && (snapshot.outputRevision == 0 || outputsChanged))) {
    ++snapshot.outputRevision;
  }
  effects.accepted = true;
  return effects;
}

const char* thresholdStorageKey(OutputChannel channel, CommandSource source) {
  if (source != CommandSource::Web && source != CommandSource::Blynk) return nullptr;
  switch (channel) {
    case OutputChannel::Temperature: return "NhietDo";
    case OutputChannel::Humidity: return "DoAm";
    // Preserve the existing keys; correcting this discrepancy is a separate change.
    case OutputChannel::Dust: return source == CommandSource::Web ? "DoBui" : "Bui";
  }
  return nullptr;
}
