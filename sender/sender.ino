///station que envia os comandos para o robô - 1 fitas

#define ROBOT_PASSWORD 2400

#include <esp_now.h>
#include <WiFi.h>

uint8_t mac_address_robot[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //mac address do robo


esp_now_peer_info_t peer;

const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];   
boolean newData = false;     
int id, count;

typedef struct struct_message {
  int password;
  char message;
} struct_message;

/*typedef struct struct_message {
  int data;
} struct_message;*/

struct_message commands;

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
  //commands.password = ROBOT_PASSWORD;
  commands.data = 0;
}

void loop() {
  //commands.message = "aaa";
  recvWithStartEndMarkers()
  
  if (newData == true){
      strcpy(commands.message, receivedChars);
      commands.password = ROBOT_PASSWORD;
      newData = false;
      esp_err_t result = esp_now_send(mac_address_robot, (uint8_t *) &DataToSend, sizeof(DataToSend));
      delay(3);
  }
 // commands.data = 1;
  //esp_err_t result = esp_now_send(mac_address_robot, /(uint8_t *) &commands, sizeof(commands));
}

void recvWithStartEndMarkers(){
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char in;

    while (Serial.available()){
        //  Formato da mensagem::
        // <[id1],[v_x1],[v_l1],[kick_straight],[kick_dug1],[theta1: float], [dribler1: bool], [
        //  [id2],[v_x2],[v_l2],[kick2],[theta2], [kick_dug2]
        //  [id3],[v_x3],[v_l3],[kick3],[theta3]>
        in = Serial.read();

        if (recvInProgress == true){
            if (in != endMarker){
                receivedChars[ndx] = in;
                ndx++;
                if (ndx >= numChars){
                    ndx = numChars - 1;
                }
            }
            else{
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (in == startMarker){
            recvInProgress = true;
        }
    }
}


/*
            {
                robot_id: int,
                color: 'yellow|blue',
                vx: float,
                vy: float,
                can_kick: bool,
                actual_theta: float
                
            }
*/