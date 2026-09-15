#include "tasks/communication/async_dns.h"
#include <Arduino.h>
#include <lwip/dns.h>
#include <lwip/tcpip.h>
#include <cstring>

bool AsyncDns::begin() {
  if (results) return true;
  results = xQueueCreate(1, sizeof(Result));
  request.results = results;
  return results != nullptr;
}

void AsyncDns::dnsFound(const char*, const ip_addr_t* address, void* context) {
  const auto* work = static_cast<const Request*>(context);
  const Result result{work->generation,
      address && IP_IS_V4(address) ? ip4_addr_get_u32(ip_2_ip4(address)) : 0};
  // One outstanding request, one result slot; no pointer to caller stack escapes.
  xQueueOverwrite(work->results, &result);
}

void AsyncDns::startLookup(void* context) {
  auto* work = static_cast<Request*>(context);
  ip_addr_t address;
  const err_t status = dns_gethostbyname(work->host, &address, dnsFound, work);
  if (status == ERR_OK) dnsFound(nullptr, &address, work);
  else if (status != ERR_INPROGRESS) dnsFound(nullptr, nullptr, work);
}

void AsyncDns::invalidate() {
  ++generation;
  cached = false;
  // Do not reuse a pending request: its callback can still reference it.
}

bool AsyncDns::lookup(const char* host, IPAddress& address) {
  if (!results || !host || host[0] == '\0' || std::strlen(host) >= sizeof request.host) return false;
  if (address.fromString(host)) return true;
  const uint32_t now = millis();
  Result result;
  if (pending && xQueueReceive(results, &result, 0) == pdPASS) {
    pending = false;
    if (result.generation == generation && result.address != 0) {
      std::strcpy(cachedHost, request.host);
      cachedAddress = result.address;
      cachedAtMs = now;
      cached = true;
    } else {
      Serial.println("event=dns_result status=failed_or_obsolete");
    }
  }
  if (cached && std::strcmp(cachedHost, host) == 0 &&
      static_cast<uint32_t>(now - cachedAtMs) < 60000) {
    address = IPAddress(cachedAddress);
    return true;
  }
  if (pending) {
    if (!timeoutLogged && static_cast<uint32_t>(now - startedAtMs) >= 2000) {
      Serial.println("event=dns_pending budget_ms=2000 action=continue_local_work");
      timeoutLogged = true;
    }
    return false;
  }
  std::strcpy(request.host, host);
  request.generation = generation;
  startedAtMs = now;
  timeoutLogged = false;
  pending = true;
  // Dispatch from CommunicationTask to lwIP's TCP/IP task without waiting for DNS.
  if (tcpip_try_callback(startLookup, &request) != ERR_OK) {
    pending = false;
    Serial.println("event=dns_request_rejected reason=tcpip_queue_full");
  }
  return false;
}
