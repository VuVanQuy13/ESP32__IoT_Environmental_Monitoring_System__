#pragma once

#include <IPAddress.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <lwip/ip_addr.h>

// Persistent transport-owned resolver. Requests/results cross TCP/IP task by Queue.
// Lifetime must extend until shutdown: lwIP may deliver a late DNS callback.
class AsyncDns {
 public:
  bool begin();
  bool lookup(const char* host, IPAddress& address);
  void invalidate();
 private:
  struct Result {uint32_t generation; uint32_t address;};
  struct Request {char host[254]{}; QueueHandle_t results = nullptr; uint32_t generation = 0;};
  Request request;
  QueueHandle_t results = nullptr;
  bool pending = false;
  bool timeoutLogged = false;
  bool cached = false;
  uint32_t generation = 0;
  uint32_t startedAtMs = 0;
  uint32_t cachedAtMs = 0;
  uint32_t cachedAddress = 0;
  char cachedHost[254]{};
  static void startLookup(void* context);
  static void dnsFound(const char*, const ip_addr_t* address, void* context);
};
