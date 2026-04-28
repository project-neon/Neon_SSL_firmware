#include "esp_now.h"
#include <WiFi.h>

typedef struct struct_data {
    int password;
    char message[numChars];
} struct_data;
  
typedef struct struct_feedback {
    int password;
    int id;
    int rssi;
    float battery; 
} struct_feedback;

struct_data DataReceived;
struct_feedback DataFeedback;
esp_now_peer_info_t peer;
esp_now_send_status_t last_status = ESP_NOW_SEND_FAIL;
esp_err_t err;
bool paused = false;
int fail_streak = 0;


void OnDataSent(const uint8_t* mac, esp_now_send_status_t status) {
    last_status = status;
    if (status == ESP_NOW_SEND_SUCCESS) fail_streak = 0;
    else if (fail_streak < 255)    fail_streak++;
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&DataReceived, incomingData, sizeof(DataReceived));
    if (DataReceived.password != ROBOT_PASSWORD) return;
    first_mark = millis();
    strcpy(commands, DataReceived.message);
    new_data=true;
}

bool send_feedback() {
    DataFeedback.password = FB_PASSWORD;
    DataFeedback.rssi     = rssi;
    DataFeedback.id       = robot_id;
    DataFeedback.battery  = readBattery();

    esp_err_t e = esp_now_send(mac_address_feedback, (uint8_t*)&DataFeedback, sizeof(DataFeedback));
    if (e != ESP_OK) {
        if (fail_streak < 255) fail_streak++;
        return false;
    }
    return true;
}

void handle_feedback(bool enabled) {
    if (!enabled) { 
      paused = false; 
      fail_streak = 0; 
      return;
    }
    const uint32_t now = millis();

    if (!paused && fail_streak >= MAX_FAILS_BEFORE_PAUSE) {
        paused        = true;
        pause_until_ms = now + PAUSE_COOLDOWN_MS;
    }

    if (!paused) {
        if (now - last_feedback_ms>= FEEDBACK_PERIOD_MS) {
            if (send_feedback()) last_feedback_ms = now;
        }
        return;
    }

    if (now >= pause_until_ms) {
        if (now - last_probe_ms >= PROBE_PERIOD_MS) {
            if (send_feedback()) last_probe_ms = now;
            if (last_status == ESP_NOW_SEND_SUCCESS) {
                paused      = false;
                fail_streak = 0;
                last_feedback_ms = now;
            }
        }
    }
}


void parseData(){
  char * strtokIndx;
  strtokIndx = strtok(tempChars, ",");
    
  while (strtokIndx != NULL){
    id = atoi(strtokIndx);
    
    if(id == robot_id){ 
      new_data=0;
      stop = 0;
      
      strtokIndx = strtok(NULL, ",");  
      v_l = atof(strtokIndx);       
      strtokIndx = strtok(NULL, ",");         
      v_a = atof(strtokIndx);
      strtokIndx = strtok(NULL, ",");         
      th = atof(strtokIndx);
      strtokIndx = strtok(NULL, ","); 
      if(!waiting_to_kick) kick_time = atof(strtokIndx);
      strtokIndx = strtok(NULL, ",");
    }

    else{
      strtokIndx = strtok(NULL, ",");     
      strtokIndx = strtok(NULL, ",");         
      strtokIndx = strtok(NULL, ",");         
      strtokIndx = strtok(NULL, ","); 
      strtokIndx = strtok(NULL, ",");
    }
  }   
}
  
