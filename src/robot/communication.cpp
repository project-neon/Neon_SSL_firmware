#include "communication.h"

#include <Arduino.h>

#include "robot_context.h"

void OnDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  robot->handle_send_status(status);
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  robot->handle_received_packet(incomingData, len);
}
