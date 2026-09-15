#pragma once

#include "tasks/communication/async_dns.h"
#include <WiFiClient.h>

// Used only by the Blynk transport, not by ESPAsyncWebServer.
class BoundedWiFiClient : public WiFiClient {
 public:
  bool beginDns() {return dns.begin();}
  void invalidateDns() {dns.invalidate();}
  int connect(const char* host, uint16_t port) override;
  int connect(IPAddress address, uint16_t port) override;
  int setTimeout(uint32_t) override;
  size_t write(const uint8_t* buffer, size_t length) override;
  using WiFiClient::write;
 private:
  AsyncDns dns;
};
