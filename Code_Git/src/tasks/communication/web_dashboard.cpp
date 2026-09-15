#include "tasks/communication/web_dashboard.h"
#include "shared/app_queues.h"
#include "tasks/communication/web_command.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

namespace {
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Callback boundary: copy a command into the queue, never block the network task.
bool submitCommand(const ControlEvent& event) {
  if (postControlEvent(event)) return true;
  Serial.printf("event=command_rejected reason=queue_full type=%d source=%d\n",
                static_cast<int>(event.type), static_cast<int>(event.source));
  return false;
}

void handleWebSocketRequest(AsyncWebSocketClient* client, void* arg,
                            uint8_t* data, size_t len) {
  const auto* info = static_cast<AwsFrameInfo*>(arg);
  ControlEvent event;
  if (!info || !info->final || info->index != 0 || info->len != len ||
      info->opcode != WS_TEXT || !parseWebCommand(reinterpret_cast<const char*>(data), len, event)) {
    Serial.println("event=web_command_rejected reason=invalid_frame");
    client->text("{\"error\":\"invalid_command\"}");
    return;
  }
  if (!submitCommand(event)) client->text("{\"error\":\"command_queue_full\"}");
}

void onWebSocketEvent(AsyncWebSocket*, AsyncWebSocketClient* client, AwsEventType type,
                      void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: {
      Serial.printf("event=web_connected client=%u\n", client->id());
      ControlEvent event;
      event.source = CommandSource::Web;
      if (!submitCommand(event)) client->text("{\"error\":\"command_queue_full\"}");
      break;
    }
    case WS_EVT_DATA: handleWebSocketRequest(client, arg, data, len); break;
    case WS_EVT_DISCONNECT:
      Serial.printf("event=web_disconnected client=%u\n", client->id());
      break;
    default: break;
  }
}

}

void publishWebThresholds(const Thresholds& thresholds) {
  // JSON objects are local to the communication owner, never shared with callbacks.
  JSONVar values;
  values["sensor1"] = String(thresholds.temperature);
  values["sensor2"] = String(thresholds.humidity);
  values["sensor3"] = String(thresholds.dust);
  ws.textAll(JSON.stringify(values));
}

void publishWebTelemetry(const SensorReadings& readings) {
  JSONVar values;
  values["temperature"] = readings.temperature;
  values["humidity"] = readings.humidity;
  values["dust"] = readings.dust;
  ws.textAll(JSON.stringify(values));
}

void initializeWebDashboard() {
  if (!SPIFFS.begin()) Serial.println("event=spiffs_mount_failed");
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (SPIFFS.exists("/index.html")) request->send(SPIFFS, "/index.html", "text/html");
    else request->send(200, "text/html", "<h1>No index.html found</h1>");
  });
  server.serveStatic("/", SPIFFS, "/");
  server.begin();

}

void serviceWebDashboard() { ws.cleanupClients(); }
