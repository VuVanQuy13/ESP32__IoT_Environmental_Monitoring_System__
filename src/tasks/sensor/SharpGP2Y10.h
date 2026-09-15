#ifndef SharpGP2Y10_h
#define SharpGP2Y10_h

#include <Arduino.h>
#include <Wire.h>

class SharpGP2Y10 {
  private:
    int measurePin;
    int ledPower;
    int samplingTime;
    int deltaTime;
    int sleepTime;

  public:
    // Hàm khởi tạo
    SharpGP2Y10(int measurePin, int ledPower);

    // Khởi động
    void begin();

    // Đọc giá trị ADC thô
    int readRaw();

    // Đọc điện áp đầu ra
    float readVoltage();

    // Đọc mật độ bụi (mg/m3)
    float readDustDensity();
};

#endif
