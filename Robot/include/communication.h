#pragma once

#include <esp_now.h>
#include <WiFi.h>

void OnDataSent(const uint8_t* mac, esp_now_send_status_t status);
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len);
