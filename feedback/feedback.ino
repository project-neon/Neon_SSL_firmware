///Esp q recebe os dados do robo - 2 fitas - com9 - A4:CF:12:72:B7:20

#define REFRESH_RATE 500
#define FB_PASSWORD 1500
#define DISCONNECT_TIMEOUT_MS 8000

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

int ids_connected[6] = {-1, -1 ,-1, -1, -1, -1};
unsigned long last_seen_ms[6] = { 0, 0, 0, 0, 0, 0 };


const byte num_chars = 64;

String last_id0_msg = "<-1,-1,-1.0>";
String last_id1_msg = "<-1,-1,-1.0>";
String last_id2_msg = "<-1,-1,-1.0>";
String last_id3_msg = "<-1,-1,-1.0>";
String last_id4_msg = "<-1,-1,-1.0>";
String last_id5_msg = "<-1,-1,-1.0>";

fb_message FeedbackData;

void updateIdsConnected(int id){
  if (id >= 0 && id < 6) ids_connected[id] = id;
}


void updateNumberOfConnections() {
  int count = 0;
  for (int j = 0; j < 6; j++) {
    if (ids_connected[j] != -1) count++;
  }
  n_robots = count;
}


void checkTimeouts() {
  unsigned long now = millis();
  for (int id = 0; id < 6; id++) {
    if (ids_connected[id] != -1) {
      if (now - last_seen_ms[id] > DISCONNECT_TIMEOUT_MS) {
        ids_connected[id] = -1;
        resetLastMsg(id);
      }
    }
  }
  updateNumberOfConnections();
}

void resetLastMsg(int id){
  String msg = "<-1,-1,-1.0>";
  if      (id == 0) last_id0_msg = msg;
  else if (id == 1) last_id1_msg = msg;
  else if (id == 2) last_id2_msg = msg;
  else if (id == 3) last_id3_msg = msg;
  else if (id == 4) last_id4_msg = msg;
  else if (id == 5) last_id5_msg = msg;
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
  if(FeedbackData.password == FB_PASSWORD){
    int new_id = FeedbackData.id;
    if (new_id >= 0 && new_id < 6) {
      last_seen_ms[new_id] = millis();
      updateIdsConnected(new_id);
      updateLastMsgReceived(new_id, FeedbackData.rssi, FeedbackData.battery);
      updateNumberOfConnections();
      }
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
  checkTimeouts();

  if (n_robots != 0){
    String message = last_id0_msg + last_id1_msg + last_id2_msg + last_id3_msg + last_id4_msg + last_id5_msg;
    Serial.flush(); 
    Serial.println(message);
  }

}
