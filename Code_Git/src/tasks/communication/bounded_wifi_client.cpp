#include "tasks/communication/bounded_wifi_client.h"
#include <Arduino.h>
#include <lwip/sockets.h>

int BoundedWiFiClient::connect(const char* host, uint16_t port) {
  IPAddress address;
  if (!dns.lookup(host, address)) return 0;
  return connect(address, port);
}

int BoundedWiFiClient::connect(IPAddress address, uint16_t port) {
  return WiFiClient::connect(address, port, 750);
}

int BoundedWiFiClient::setTimeout(uint32_t) {
  // Arduino-ESP32 2.0.17 expects seconds here; Blynk passes milliseconds.
  // Clamp this transport's read/socket timeout to one second, including constructor calls.
  return WiFiClient::setTimeout(1);
}

size_t BoundedWiFiClient::write(const uint8_t* buffer, size_t length) {
  if (length == 0) return 0;
  if (!buffer || fd() < 0) return 0;
  // Avoid WiFiClient's multi-retry write loop. Congestion closes this cloud session;
  // the outbox will resend the latest state on a later, paced reconnect.
  const int written = lwip_send(fd(), buffer, length, MSG_DONTWAIT);
  if (written <= 0 || static_cast<size_t>(written) != length) {
    Serial.println("event=cloud_write_incomplete action=disconnect");
    stop();
  }
  return written > 0 ? static_cast<size_t>(written) : 0;
}
