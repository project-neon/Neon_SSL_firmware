///station que envia os comandos para o robô - 1 fitas

#include <esp_now.h>
#include <WiFi.h>



//cabo branco: EC:62:60:94:CF:10
uint8_t mac_address_robot[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //mac address do robo


esp_now_peer_info_t peer;


typedef struct struct_message {
  int data;
} struct_message;

struct_message data_to_send;

void OnDataSent(const uint8_t *mac_address, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS){
      Serial.println("Mensagem enviada para o robo");
  }else{
      Serial.println("Erro ao enviar mensagem para o robo");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  else Serial.println("ESPNOW OK ");
  peer.channel = 0;  
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_address_robot, 6);
  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
    ESP.restart();
  }
  esp_now_register_send_cb(OnDataSent);
  data_to_send.data = 0;
}

void loop() {
  data_to_send.data = 1;
  esp_err_t result = esp_now_send(mac_address_robot, (uint8_t *) &data_to_send, sizeof(data_to_send));
}
