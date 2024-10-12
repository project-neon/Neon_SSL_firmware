///Esp q recebe os dados do robo - 2 fitas

#include <esp_now.h>
#include <WiFi.h>

//PENSAR EM COMO ORGANIZAR A RECEPCAO DOS DADOS DOS 3 ROBOS

typedef struct struct_message {
  int data_return;
} struct_message;

struct_message FeedbackData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&FeedbackData, incomingData, sizeof(FeedbackData));
  Serial.print("Message received: ");
  Serial.println(FeedbackData.data_return);
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
