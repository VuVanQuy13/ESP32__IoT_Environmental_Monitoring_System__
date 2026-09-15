#pragma once
#include "shared/app_types.h"

void initializeWebDashboard();
void serviceWebDashboard();
void publishWebThresholds(const Thresholds& thresholds);
void publishWebTelemetry(const SensorReadings& readings);
