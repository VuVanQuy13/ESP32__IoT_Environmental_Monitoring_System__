#include <Arduino.h>
#include <esp_task_wdt.h>
#include <freertos/task.h>
#include "shared/app_queues.h"
#include "tasks/sensor/sensor_task.h"
#include "tasks/control/control_task.h"
#include "tasks/communication/communication_task.h"
#include "shared/task_support.h"

void setup() {
  Serial.begin(9600);
  delay(1000);
  requireTaskResource(esp_task_wdt_init(15, true) == ESP_OK, "wdt_init");
  requireTaskResource(createAppQueues(), "queue_create");
  // Local acquisition/control start regardless of Wi-Fi or Blynk availability.
  requireTaskResource(startControlTask(), "control_task_create");
  requireTaskResource(startSensorTask(), "sensor_task_create");
  requireTaskResource(startCommunicationTask(), "communication_task_create");
}

void loop() {
  // Arduino still owns loopTask; the three application tasks do the actual work.
  // Each application task registers/resets its own watchdog.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
