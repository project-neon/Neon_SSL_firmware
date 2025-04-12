///ESP do ROBO - 3 fita

#include <esp_now.h>
#include <WiFi.h>


typedef struct struct_data {
  int data_received;
} struct_data;


struct_data DataReceived;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&DataReceived, incomingData, sizeof(DataReceived));
  Serial.print("Message received: ");
  Serial.println(DataReceived.data_received);
}


void setup() {
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

}