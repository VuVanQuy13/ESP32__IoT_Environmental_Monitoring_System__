#pragma once

#include "shared/app_types.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

constexpr UBaseType_t CONTROL_QUEUE_LENGTH = 16;

bool createAppQueues();  // setup only, before callbacks/tasks are enabled
bool postControlEvent(const ControlEvent& event, TickType_t waitTicks = 0);
bool receiveControlEvent(ControlEvent& event, TickType_t waitTicks);
bool publishControlSnapshot(const ControlSnapshot& snapshot);
bool peekControlSnapshot(ControlSnapshot& snapshot);
bool publishTelemetryFrame(const TelemetryFrame& frame);
bool takeTelemetryFrame(TelemetryFrame& frame);
