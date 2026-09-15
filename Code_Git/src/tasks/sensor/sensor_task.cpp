#include "tasks/sensor/sensor_task.h"
#include "shared/app_queues.h"
#include "shared/task_support.h"
#include "tasks/sensor/SharpGP2Y10.h"
#include <DHT.h>
#include <esp_task_wdt.h>
#include <freertos/task.h>

namespace {
constexpr int DHT_PIN = 13;
constexpr int DUST_ADC_PIN = 34;
constexpr int DUST_LED_PIN = 25;
constexpr uint32_t SENSOR_STACK_BYTES = 4096;
constexpr uint32_t SENSOR_PAUSE_MS = 10;
TaskHandle_t sensorHandle = nullptr;

void sensorTask(void*) {
  // These drivers are constructed and accessed by SensorTask only.
  DHT dht(DHT_PIN, DHT11);
  SharpGP2Y10 dustSensor(DUST_ADC_PIN, DUST_LED_PIN);
  dht.begin();
  dustSensor.begin();
  requireTaskResource(esp_task_wdt_add(nullptr) == ESP_OK, "sensor_wdt_add");
  Serial.printf("event=task_started task=SensorTask core=%d stack_bytes=%u\n",
                xPortGetCoreID(), SENSOR_STACK_BYTES);
  uint32_t lastLog = 0;
  for (;;) {
    ControlEvent event;
    event.type = EventType::Sample;
    event.readings = {dht.readTemperature(), dht.readHumidity(), dustSensor.readDustDensity()};
    event.sampledAtMs = millis();
    if (!postControlEvent(event, pdMS_TO_TICKS(100))) {
      Serial.println("event=sample_dropped reason=control_queue_full");
    }
    if (static_cast<uint32_t>(event.sampledAtMs - lastLog) >= 2000) {
      lastLog = event.sampledAtMs;
      Serial.printf("event=sensor_sample temperature=%.1f humidity=%.1f dust=%.2f\n",
                    event.readings.temperature, event.readings.humidity, event.readings.dust);
    }
    requireTaskResource(esp_task_wdt_reset() == ESP_OK, "sensor_wdt_reset");
    // Driver retains its microsecond waveform. This pause yields AFTER a full read.
    // Not a 2-second sample period: DHT caches for 2 s internally; dust is read each pass.
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PAUSE_MS));
  }
}
}

bool startSensorTask() {
  if (sensorHandle != nullptr) return false;
  // Keep timing-sensitive acquisition on the Arduino application core (ESP32: 1).
  return xTaskCreatePinnedToCore(sensorTask, "SensorTask", SENSOR_STACK_BYTES, nullptr,
                                 1, &sensorHandle, ARDUINO_RUNNING_CORE) == pdPASS;
}
