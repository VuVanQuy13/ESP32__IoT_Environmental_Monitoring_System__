#include "tasks/communication/web_command.h"
#include <limits.h>
#include <string.h>

bool parseWebCommand(const char* data, size_t length, ControlEvent& event) {
  if (data == nullptr) return false;
  ControlEvent parsed;
  parsed.source = CommandSource::Web;
  if (length == 9 && memcmp(data, "getValues", 9) == 0) {
    event = parsed;
    return true;
  }
  // Longest supported command: 3s-2147483648 (13 bytes on ESP32).
  if (length < 3 || length > 13 || data[0] < '1' || data[0] > '3' || data[1] != 's') {
    return false;
  }
  size_t pos = 2;
  const bool negative = data[pos] == '-';
  if (negative || data[pos] == '+') ++pos;
  if (pos == length) return false;
  unsigned long magnitude = 0;
  const unsigned long limit = negative ? static_cast<unsigned long>(INT_MAX) + 1UL : INT_MAX;
  for (; pos < length; ++pos) {
    if (data[pos] < '0' || data[pos] > '9') return false;
    const unsigned digit = static_cast<unsigned>(data[pos] - '0');
    if (magnitude > (limit - digit) / 10) return false;
    magnitude = magnitude * 10 + digit;
  }
  parsed.type = EventType::SetThreshold;
  parsed.channel = static_cast<OutputChannel>(data[0] - '1');
  parsed.value = negative ? (magnitude == limit ? INT_MIN : -static_cast<int>(magnitude))
                          : static_cast<int>(magnitude);
  event = parsed;
  return true;
}
