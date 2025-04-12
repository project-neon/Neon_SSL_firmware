#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

typedef struct struct_message {
  int time;
} struct_message;

esp_now_peer_info_t peer;

struct_message commands;

uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //mac address do robo

void OnDataSent(const uint8_t *mac_address, esp_now_send_status_t status) {
    digitalWrite(2,HIGH);
    delay(3);
    digitalWrite(2,LOW);
}

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(2, OUTPUT);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  else Serial.println("ESPNOW OK ");

  peer.channel = 0;  
  peer.encrypt = false;

  memcpy(peer.peer_addr, broadcast_address, 6);

  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
  }
  esp_now_register_send_cb(OnDataSent);

  commands.time = 0;

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    int time_us = Serial.parseInt();
    commands.time = time_us;
    Serial.println("value received");
    esp_err_t message = esp_now_send(broadcast_address, (uint8_t *) &commands, sizeof(commands));
  }

}
