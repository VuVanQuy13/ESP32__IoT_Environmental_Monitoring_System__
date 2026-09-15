#include "tasks/communication/communication_task.h"
#include "tasks/communication/communication_state.h"
#include "shared/app_queues.h"
#include "tasks/communication/network_connection.h"
#include "tasks/communication/web_dashboard.h"
#include "shared/task_support.h"
#include <esp_task_wdt.h>
#include <freertos/task.h>

namespace {
constexpr uint32_t COMMUNICATION_STACK_BYTES = 8192;
TaskHandle_t communicationHandle = nullptr;

void communicationTask(void*) {
  requireTaskResource(esp_task_wdt_add(nullptr) == ESP_OK, "communication_wdt_add");
  requireTaskResource(initializeNetwork(), "network_initialize");
  initializeWebDashboard();
  Serial.printf("event=task_started task=Communication core=%d stack_bytes=%u\n",
                xPortGetCoreID(), COMMUNICATION_STACK_BYTES);
  RetrySchedule wifiRetry(WIFI_RETRY_MS);
  RetrySchedule cloudRetry(CLOUD_RETRY_MS);
  CloudOutbox outbox;
  bool wasWifiUp = false;
  bool wasCloudUp = false;
  bool webThresholdsSent = false;
  uint32_t webThresholdRevision = 0;
  TelemetryFrame latestTelemetry{};
  bool hasTelemetry = false;

  for (;;) {
    const bool wifiUp = wifiConnected();
    const bool wifiRestored = wifiUp && !wasWifiUp;
    if (wifiUp != wasWifiUp) {
      Serial.printf("event=wifi_status connected=%d\n", wifiUp);
      cloudRetry.reset();
      webThresholdsSent = false;
      if (!wifiUp) disconnectCloud(true);
      wasWifiUp = wifiUp;
    }
    if (!wifiUp) {
      if (wifiRetry.due(millis())) {
        const bool requested = requestWifiConnection();
        wifiRetry.attempted(millis());
        Serial.printf("event=wifi_attempt requested=%d retry_ms=%u\n", requested, WIFI_RETRY_MS);
      }
    } else {
      if (!cloudConnected() && cloudRetry.due(millis())) {
        const bool connected = tryCloudConnection(CLOUD_CONNECT_BUDGET_MS);
        cloudRetry.attempted(millis()); // retry spacing starts AFTER this attempt finishes
        Serial.printf("event=cloud_attempt connected=%d retry_ms=%u\n", connected, CLOUD_RETRY_MS);
      }
      if (cloudConnected()) serviceCloudConnection();
    }

    const bool cloudUp = wifiUp && cloudConnected();
    if (cloudUp != wasCloudUp) {
      Serial.printf("event=cloud_status connected=%d\n", cloudUp);
      if (cloudUp) outbox.newSession();
      else {
        disconnectCloud(false);
        cloudRetry.attempted(millis());
      }
      wasCloudUp = cloudUp;
    }

    // Always drain the telemetry mailbox, even offline; keep only the newest pair.
    const bool newTelemetry = takeTelemetryFrame(latestTelemetry);
    if (newTelemetry) {
      hasTelemetry = true;
      outbox.stageTelemetry(latestTelemetry);
    }
    if (wifiUp && hasTelemetry && (newTelemetry || wifiRestored)) {
      publishWebTelemetry(latestTelemetry.web);
    }
    ControlSnapshot snapshot;
    if (peekControlSnapshot(snapshot)) {
      outbox.stageSnapshot(snapshot);
      if (wifiUp && (!webThresholdsSent || snapshot.thresholdRevision != webThresholdRevision)) {
        publishWebThresholds(snapshot.thresholds);
        webThresholdRevision = snapshot.thresholdRevision;
        webThresholdsSent = true;
      }
    }
    if (cloudUp) {
      uint8_t pin;
      double value;
      if (outbox.next(millis(), pin, value)) {
        if (publishCloudValue(pin, value)) outbox.sent(pin, millis());
        else {
          disconnectCloud(false);
          cloudRetry.attempted(millis());
          wasCloudUp = false;
          Serial.println("event=cloud_publish_failed action=retry_later");
        }
      }
    }
    serviceWebDashboard();
    requireTaskResource(esp_task_wdt_reset() == ESP_OK, "communication_wdt_reset");
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
}

bool startCommunicationTask() {
  if (communicationHandle != nullptr) return false;
  return xTaskCreatePinnedToCore(communicationTask, "Communication", COMMUNICATION_STACK_BYTES,
                                 nullptr, 1, &communicationHandle, ARDUINO_RUNNING_CORE) == pdPASS;
}
