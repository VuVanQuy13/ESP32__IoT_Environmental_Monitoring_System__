#include "tasks/sensor/SharpGP2Y10.h"

SharpGP2Y10::SharpGP2Y10(int mPin, int lPin) {
  measurePin = mPin;
  ledPower   = lPin;
  samplingTime = 280;
  deltaTime   = 40;
  sleepTime   = 9680;
}

void SharpGP2Y10::begin() {
  pinMode(ledPower, OUTPUT);
}


int SharpGP2Y10::readRaw() {
  // Điều khiển LED của cảm biến
  digitalWrite(ledPower, LOW);                 // Bật LED (active LOW)
  delayMicroseconds(samplingTime);

  // Đọc giá trị Analog từ cảm biến
  int voMeasured = analogRead(measurePin);    // Đọc ADC
  delayMicroseconds(deltaTime);

  // Tắt LED hồng ngoại
  digitalWrite(ledPower, HIGH);                // Tắt LED
  delayMicroseconds(sleepTime);
  
  return voMeasured;
}

// Lấy giá trị Vôn
float SharpGP2Y10::readVoltage() {
  int raw = readRaw();
  return raw * (3.3 / 4096);   // Nếu dùng 3.3V thì sửa thành (3.3 / 4096.0) cho ESP32
}

// Lấy giá trị bụi
float SharpGP2Y10::readDustDensity() {
  float voltage = readVoltage();
  float dustDensity = (voltage - 0.6) / 0.5;
  return dustDensity > 0 ? dustDensity : 0;  
}
