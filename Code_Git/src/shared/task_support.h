#pragma once

#include <Arduino.h>
#include <stdlib.h>

inline void requireTaskResource(bool success, const char* operation) {
  if (!success) {
    Serial.printf("event=rtos_fatal operation=%s\n", operation);
    abort();
  }
}
