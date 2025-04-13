#include <esp_now.h>
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

void OnDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
    memcpy(&DataReceived, incomingData, sizeof(DataReceived));
  
    if (DataReceived.password != ROBOT_PASSWORD) return;
  
    first_mark = millis();
    strcpy(commands, DataReceived.message);
    new_data=1;
  }
  
void sendFeedback() {
    DataFeedback.password = FB_PASSWORD;
    DataFeedback.rssi = rssi;
    DataFeedback.id = robot_id;
    DataFeedback.battery = readBattery();
    esp_now_send(mac_address_feedback, (uint8_t *) &DataFeedback, sizeof(DataFeedback));
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
      kick_time = atof(strtokIndx);
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
  