///station que envia os comandos para o robô - 1 fitas

#define ROBOT_PASSWORD 2400

#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //mac address do robo


esp_now_peer_info_t peer;

const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];   
boolean newData = false;     
int id, count;

typedef struct struct_message {
  int password;
  char message[numChars];
} struct_message;


struct_message commands;


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(2, OUTPUT);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }

  else Serial.println("ESPNOW OK ");
  peer.channel = 0;  
  peer.encrypt = false;
  memcpy(peer.peer_addr, broadcast_address, 6);
  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
    ESP.restart();
  }
}

void loop() {
  recvWithStartEndMarkers();
  
  if (newData == true){
      strcpy(commands.message, receivedChars);
      commands.password = ROBOT_PASSWORD;
      sendData();
      newData = false;
  }
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


void sendData(){   
    // esse delay é necessário para que os dados sejam enviados corretamente
    esp_err_t message = esp_now_send(broadcast_address, (uint8_t *) &commands, sizeof(commands));
    digitalWrite(2,HIGH);
    delay(3);
    digitalWrite(2,LOW);
}