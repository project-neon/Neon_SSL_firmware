// ///station que envia os comandos para o robô - numero 1 - com9
// // <2,0.5,1.0,0.0,300>
// //CC:DB:A7:3F:B6:4C
// #define ROBOT_PASSWORD 2400

// #include <esp_now.h>
// #include <WiFi.h>


// uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //mac address do robo


// esp_now_peer_info_t peer;

// const byte numChars = 200;
// char receivedChars[numChars];
// char tempChars[numChars];   
// boolean newData = false;     
// int id, count;


// typedef struct struct_message {
//   int password;
//   char message[numChars];
// } struct_message;


// struct_message commands;


// void setup() {
//   Serial.begin(115200);
//   WiFi.mode(WIFI_STA);

//   pinMode(2, OUTPUT);

//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     ESP.restart();
//     return;
//   }

//   else Serial.println("ESPNOW OK ");
//   peer.channel = 0;  
//   peer.encrypt = false;
//   memcpy(peer.peer_addr, broadcast_address, 6);
//   if (esp_now_add_peer(&peer) != ESP_OK){
//     Serial.println("Failed to add peer");
//     ESP.restart();
//   }
// }

// void loop() {
//   recvWithStartEndMarkers();
  
//   if (newData == true){
//       strcpy(commands.message, receivedChars);
//       commands.password = ROBOT_PASSWORD;
//       sendData();
//       newData = false;
//   }
// }

// void recvWithStartEndMarkers(){
//     static boolean recvInProgress = false;
//     static byte ndx = 0;
//     char startMarker = '<';
//     char endMarker = '>';
//     char in;

//     while (Serial.available()){
//         //  Formato da mensagem::
//         // <[id1],[v_x1],[v_l1],[kick_straight],[kick_dug1],[theta1: float], [dribler1: bool], [
//         //  [id2],[v_x2],[v_l2],[kick2],[theta2], [kick_dug2]
//         //  [id3],[v_x3],[v_l3],[kick3],[theta3]>
//         in = Serial.read();

//         if (recvInProgress == true){
//             if (in != endMarker){
//                 receivedChars[ndx] = in;
//                 ndx++;
//                 if (ndx >= numChars){
//                     ndx = numChars - 1;
//                 }
//             }
//             else{
//                 receivedChars[ndx] = '\0'; // terminate the string
//                 recvInProgress = false;
//                 ndx = 0;
//                 newData = true;
//             }
//         }

//         else if (in == startMarker){
//             recvInProgress = true;
//         }
//     }
// }


// void sendData(){   
//     // esse delay é necessário para que os dados sejam enviados corretamente
//     esp_err_t message = esp_now_send(broadcast_address, (uint8_t *) &commands, sizeof(commands));
//     digitalWrite(2,HIGH);
//     delay(3);
//     digitalWrite(2,LOW);
// }


//teste


///station que envia os comandos para o robô - numero 1 - com9
// Formato de entrada humana: <2,0.5,1.0,0.0,300>
// Para o pacote enviado via ESP-NOW, vai SEM os marcadores: "2,0.5,1.0,0.0,300"

#define ROBOT_PASSWORD 2400

#include <esp_now.h>
#include <WiFi.h>

// ====== CONFIG AUTO TEST ======
#define AUTO_MODE        1        // 1=ativa o envio automático; 0=somente Serial
#define AUTO_PERIOD_MS   200      // período entre envios automáticos
const char* DEFAULT_CMD = "2,0.5,1.0,0.0,300";
// ==============================

uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // broadcast

esp_now_peer_info_t peer;

const byte numChars = 200;
char receivedChars[numChars];
boolean newData = false;

typedef struct struct_message {
  int password;
  char message[numChars];
} struct_message;

struct_message commands;

void recvWithStartEndMarkers();
void sendData();

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(2, OUTPUT);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  } else {
    Serial.println("ESPNOW OK");
  }

  memset(&peer, 0, sizeof(peer));
  memcpy(peer.peer_addr, broadcast_address, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
  }

  Serial.print("AUTO_MODE: ");
  Serial.println(AUTO_MODE ? "ON" : "OFF");
}

void loop() {
  // 1) Sempre escuta Serial (tem prioridade sobre o AUTO se chegar algo)
  recvWithStartEndMarkers();
  if (newData) {
    strncpy(commands.message, receivedChars, numChars - 1);
    commands.message[numChars - 1] = '\0';
    commands.password = ROBOT_PASSWORD;
    sendData();
    newData = false;
    return; // já enviou, não manda o automático neste ciclo
  }

#if AUTO_MODE
  // 2) Envio automático periódico do DEFAULT_CMD
  static uint32_t last_auto = 0;
  uint32_t now = millis();
  if (now - last_auto >= AUTO_PERIOD_MS) {
    strncpy(commands.message, DEFAULT_CMD, numChars - 1);
    commands.message[numChars - 1] = '\0';
    commands.password = ROBOT_PASSWORD;
    sendData();
    last_auto = now;
  }
#endif
}

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  const char startMarker = '<';
  const char endMarker   = '>';

  while (Serial.available()) {
    char in = Serial.read();
    if (recvInProgress) {
      if (in != endMarker) {
        if (ndx < numChars - 1) {
          receivedChars[ndx++] = in;
        }
      } else {
        receivedChars[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (in == startMarker) {
      recvInProgress = true;
      ndx = 0;
    }
  }
}

void sendData() {
  // Envia pacote via ESP-NOW
  esp_err_t err = esp_now_send(broadcast_address, (uint8_t*)&commands, sizeof(commands));
  // pulso no LED para indicar envio
  digitalWrite(2, HIGH);
  delay(3);
  digitalWrite(2, LOW);

  // debug opcional
  if (err == ESP_OK) {
    Serial.print("Sent: ");
    Serial.println(commands.message);
  } else {
    Serial.print("ESP-NOW send error: ");
    Serial.println((int)err);
  }
}