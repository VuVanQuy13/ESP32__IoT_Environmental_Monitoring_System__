#include "tasks/communication/network_connection.h"
#include "tasks/communication/network_config.h"
#include "tasks/communication/bounded_wifi_client.h"
#include "shared/app_queues.h"
#include <WiFi.h>
#define NO_GLOBAL_BLYNK
#include <BlynkSimpleEsp32.h>

namespace {
BoundedWiFiClient cloudClient;
BlynkEsp32Client cloudTransport(cloudClient);
}
BlynkWifi Blynk(cloudTransport);

bool initializeNetwork() {
  if (!cloudClient.beginDns()) return false;
  if (!WiFi.mode(WIFI_STA)) return false;
  if (!WiFi.setAutoReconnect(false)) return false;
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.disconnect();
  return true;
}

bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }

bool requestWifiConnection() {
  // No wait-for-connection loop. CommunicationTask schedules the next attempt.
  if (!WiFi.disconnect(false, false)) {
    Serial.println("event=wifi_disconnect_failed action=retry_begin");
  }
  return WiFi.begin(networkConfig::ssid, networkConfig::password) != WL_CONNECT_FAILED;
}

bool cloudConnected() { return Blynk.connected() && cloudClient.connected(); }

bool tryCloudConnection(uint32_t budgetMs) {
  const bool connected = Blynk.connect(budgetMs);
  if (!connected) Blynk.disconnect();
  return connected;
}

void serviceCloudConnection() {
  if (!Blynk.run()) Blynk.disconnect();
}

void disconnectCloud(bool invalidateDns) {
  Blynk.disconnect();
  if (invalidateDns) cloudClient.invalidateDns();
}

bool publishCloudValue(uint8_t pin, double value) {
  if (pin > 12 || !cloudConnected()) return false;
  Blynk.virtualWrite(pin, value);
  // Transport acceptance, not an acknowledgement from the dashboard.
  const bool sent = cloudConnected();
  if (!sent) Blynk.disconnect();
  return sent;
}

namespace {
void submitBlynkCommand(EventType type, OutputChannel channel, int value) {
  ControlEvent event;
  event.type = type;
  event.channel = channel;
  event.value = value;
  event.source = CommandSource::Blynk;
  if (!postControlEvent(event)) {
    Serial.printf("event=command_rejected reason=queue_full type=%d source=%d\n",
                  static_cast<int>(type), static_cast<int>(event.source));
  }
}
}

BLYNK_WRITE(V6) { submitBlynkCommand(EventType::ManualOutput, OutputChannel::Temperature, param.asInt()); }
BLYNK_WRITE(V7) { submitBlynkCommand(EventType::ManualOutput, OutputChannel::Humidity, param.asInt()); }
BLYNK_WRITE(V8) { submitBlynkCommand(EventType::ManualOutput, OutputChannel::Dust, param.asInt()); }
BLYNK_WRITE(V9) { submitBlynkCommand(EventType::SetMode, OutputChannel::Temperature, param.asInt()); }
BLYNK_WRITE(V10) { submitBlynkCommand(EventType::SetThreshold, OutputChannel::Temperature, param.asInt()); }
BLYNK_WRITE(V11) { submitBlynkCommand(EventType::SetThreshold, OutputChannel::Humidity, param.asInt()); }
BLYNK_WRITE(V12) { submitBlynkCommand(EventType::SetThreshold, OutputChannel::Dust, param.asInt()); }
