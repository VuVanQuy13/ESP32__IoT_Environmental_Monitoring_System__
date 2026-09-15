#pragma once
#include <stdint.h>

bool initializeNetwork();
bool wifiConnected();
bool requestWifiConnection();
bool cloudConnected();
bool tryCloudConnection(uint32_t budgetMs);
void serviceCloudConnection();
void disconnectCloud(bool invalidateDns);
bool publishCloudValue(uint8_t pin, double value);
