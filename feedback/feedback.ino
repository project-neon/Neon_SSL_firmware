///Esp q recebe os dados do robo - 2 fitas

#define REFRESH_RATE 500

#include <esp_now.h>
#include <WiFi.h>

//PENSAR EM COMO ORGANIZAR A RECEPCAO DOS DADOS DOS 3 ROBOS

unsigned long first_mark = 0;

unsigned long second_mark = 0;

/*typedef struct struct_message {
  int id;
} fb_message;*/



typedef struct struct_message {
  int password;
  int id;
  int rssi;
  int battery;
 
 // bool sensor_kick;
} fb_message;



int n_robots = 0;

int FB_PASSWORD = 1500;

int ids_connected[3] = {0, 0 ,0};

const byte num_chars = 64;
String last_id1_msg = "<0,0,0>";
String last_id2_msg = "<0,0,0>";
String last_id3_msg = "<0,0,0>";

fb_message FeedbackData;

void updateIdsConnected(int id){
  if (id == 1) ids_connected[0] = 1;
  if (id == 2) ids_connected[1] = 2;
  if (id == 3) ids_connected[2] = 3;
}

//verifica se o id esta conectado
void updateNumberOfConnections(int id){
  if (n_robots == 0){
    updateIdsConnected(id);
    n_robots = n_robots +1;
    return;
  }
  else{ 
    for(int j = 0; j < 3; j++){
      if (ids_connected[j] == id){
        return;
      }
    }
    n_robots = n_robots + 1;
    updateIdsConnected(id);
  }
}


void updateLastMsgReceived(int id, int rssi, int battery){
  //char last_received = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 1) last_id1_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
 // if (id == 2) strcpy(last_id2_msg, last_received);
  if (id == 2) last_id2_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 3) last_id3_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
}

void OnDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
  memcpy(&FeedbackData, incomingData, sizeof(FeedbackData));
  if (FeedbackData.password == FB_PASSWORD){
    int new_id = FeedbackData.id;
    updateNumberOfConnections(new_id);
    updateLastMsgReceived(new_id, FeedbackData.rssi, FeedbackData.battery);
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
 esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  if (n_robots != 0){
    String message = last_id1_msg + last_id2_msg + last_id3_msg;
    Serial.println(message);
  }

}
