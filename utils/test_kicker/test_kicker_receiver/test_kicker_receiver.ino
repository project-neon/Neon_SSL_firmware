#define KICK_PIN 32

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

esp_now_peer_info_t peer;

//struct received
typedef struct struct_data {
  int time_us;
} struct_data;


struct_data DataReceived;

void OnDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
  digitalWrite(2,HIGH);
  delay(3);
  digitalWrite(2,LOW);

  memcpy(&DataReceived, incomingData, sizeof(DataReceived));

  int time = DataReceived.time_us;

  digitalWrite(KICK_PIN, HIGH);
  delayMicroseconds(time);
  digitalWrite(KICK_PIN, LOW);
}


void setup() {

  Serial.begin(115200);

  pinMode(KICK_PIN, OUTPUT);
  pinMode(2, OUTPUT);

  delay(100);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // put your main code here, to run repeatedly:

}
