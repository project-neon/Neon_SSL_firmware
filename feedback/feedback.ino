///Esp q recebe os dados do robo - 2 fitas

#define REFRESH_RATE 500

#include <esp_now.h>
#include <WiFi.h>


unsigned long first_mark = 0;

unsigned long second_mark = 0;

typedef struct struct_message {
  int password;
  int id;
  int rssi;
  float battery;
} fb_message;


int n_robots = 0;

int FB_PASSWORD = 1500;

int ids_connected[6] = {-1, -1 ,-1, -1, -1, -1};

const byte num_chars = 64;

String last_id0_msg = "<-1,-1,-1.0>";
String last_id1_msg = "<-1,-1,-1.0>";
String last_id2_msg = "<-1,-1,-1.0>";
String last_id3_msg = "<-1,-1,-1.0>";
String last_id4_msg = "<-1,-1,-1.0>";
String last_id5_msg = "<-1,-1,-1.0>";

fb_message FeedbackData;

void updateIdsConnected(int id){
  if (id == 0) ids_connected[0] = 0;
  if (id == 1) ids_connected[1] = 1;
  if (id == 2) ids_connected[2] = 2;
  if (id == 3) ids_connected[3] = 3;
  if (id == 4) ids_connected[4] = 4;
  if (id == 5) ids_connected[5] = 5;
}

//verifica se o id esta conectado
void updateNumberOfConnections(int id){
  if (n_robots == 0){
    updateIdsConnected(id);
    n_robots = n_robots +1;
    return;
  }
  else{ 
    for(int j = 0; j < 6; j++){
      if (ids_connected[j] == id){
        return;
      }
    }
    n_robots = n_robots + 1;
    updateIdsConnected(id);
  }
}


void updateLastMsgReceived(int id, int rssi, float battery){
  if (id == 0) last_id0_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 1) last_id1_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 2) last_id2_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 3) last_id3_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 4) last_id4_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
  if (id == 5) last_id5_msg = "<"+ String(id)+ "," + String(rssi) + "," + String(battery)+">";
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
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
    }
 esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  if (!Serial){
    Serial.begin(9600);
    delay(100);
    ESP.restart();
  }
  if (n_robots != 0){
    String message = last_id0_msg + last_id1_msg + last_id2_msg + last_id3_msg + last_id4_msg + last_id5_msg;
    Serial.flush(); 
    Serial.println(message);
  }

}
