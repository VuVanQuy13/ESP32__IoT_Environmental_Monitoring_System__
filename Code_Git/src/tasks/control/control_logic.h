#pragma once

#include "shared/app_types.h"

// Pure decisions; the caller performs GPIO, logging and network operations.
AutoDecision evaluateAuto(ControlMode mode, const SensorReadings& readings,
                          const Thresholds& thresholds);
ManualDecision evaluateManual(const ManualCommand& command);
ControlMode decodeModeCommand(int value);
