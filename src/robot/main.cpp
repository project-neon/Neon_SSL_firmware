#include <WiFi.h>

#include "communication.h"
#include "robot_context.h"
#include "sensor.h"

RobotContext *robot = nullptr;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(2, OUTPUT);
  pinMode(VOLTAGE_SENSOR_PIN, INPUT);

  const uint8_t mac_fb[6] = {0xA4, 0xCF, 0x12, 0x72, 0xB7, 0x20};
  const uint8_t mac_st[6] = {0x08, 0xB6, 0x1F, 0x28, 0xE3, 0x94};
  robot =
      new RobotContext(2, true, true, mac_fb, mac_st, 0.0785f, 0.03f, 100.0f);

  robot->setup_kicker_pins();
  robot->begin_charge();
  robot->setup_peer(0);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }

  if (esp_now_add_peer(robot->peer_info()) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  esp_wifi_set_promiscuous(robot->rssi_enabled());
  if (robot->rssi_enabled()) {
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
  }
}

void loop() {
  robot->process_command();

  int now_ms = millis();
  if (now_ms - robot->last_command_ms() > FAILSAFE_MS) {
    robot->fail_safe_check();
  }

  robot->kicker_update();
  robot->update_timing();

  if (!robot->is_stopped()) {
    robot->motors_update();
  }

  robot->feedback_update();
}
