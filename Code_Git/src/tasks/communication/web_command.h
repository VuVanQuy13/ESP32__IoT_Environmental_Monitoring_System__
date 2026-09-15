#pragma once

#include "shared/app_types.h"
#include <stddef.h>

// Accept complete getValues or <1|2|3>s<signed integer> frames only.
bool parseWebCommand(const char* data, size_t length, ControlEvent& event);
