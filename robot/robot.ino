///ESP do ROBO - 3 fita

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
//#include <ESP32Servo.h>



#define VOLTAGE_SENSOR_PIN 34
#define SENSOR_KICKER 14
#define KICKER_PIN 18
#define DRIBBLER_PIN 32
#define MAX_DRIBBLER 110 //não é definitivo

#define FREQUENCIA_DRIBBLER 50
#define MIN_THROTTLE_DRIBBLER 1048
#define MAX_THROTTLE_DRIBBER 1952

#define FB_PASSWORD 1500

//2 fitas: 08:B6:1F:28:E3:94
uint8_t mac_address_feedback[6] = {0x08, 0xB6, 0x1F, 0x28, 0xE3, 0x94}; //mac address do feedback

const byte numChars = 64;
char commands[numChars];
char tempChars[numChars];

int first_mark = 0, second_mark;
int id;
int robot_id = 2;
float v_l, v_a, theta;
bool kick;

esp_now_peer_info_t peer;

//Servo Dribbler;

int status_com = 0;
int32_t rssi = 0;

//struct received
/*typedef struct struct_data {
  int header;
  char message[numChars];
} struct_data;*/

typedef struct struct_data {
  int data_received;
} struct_data;



//struct to send

/*
- id
- rssi;
- battery
- kick;
- velocidade real;
- temperature AINDA NAO
*/

typedef struct struct_feedback {
  int password;
  int id;
  int rssi;
  //int real_speed;
  //bool sensor_kick;
  int battery; 

} struct_feedback;

/*
typedef struct struct_feedback {
  int data_feedback;
} struct_feedback;
*/

typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6];
  uint8_t addr2[6]; 
  uint8_t addr3[6];
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; 
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; 
} wifi_ieee80211_packet_t;


struct_data DataReceived;
struct_feedback DataFeedback;

void OnDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
  status_com = 1;
  memcpy(&DataReceived, incomingData, sizeof(DataReceived));
  first_mark = millis();
  Serial.print("Message received: ");
  //strcpy(commands, DataReceived.message);
//  header = DataReceived.header;
  Serial.println(DataReceived.data_received);
}

void promiscuous_rx_cb(void *buff, wifi_promiscuous_pkt_type_t type) {
  /*
  rssi < -90 dbm : muito ruim
  rssi ~ -65 dbm: sinal ok
  rssi > -55 dbm: sinal bom
  rssi > -30 dbm: sinal ótimo
  */
  if (type != WIFI_PKT_MGMT)
    return;
  else{
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    rssi = ppkt->rx_ctrl.rssi;
  }
}

// Função callback chamada ao enviar algum dado
void OnDataSent(const uint8_t *mac_address_feedback, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS){
      Serial.println("Mensagem enviada para o FB");
  }else{
      Serial.println("Erro ao enviar mensage para o FB");
  }
}

void send_power(float m1,float m2,float m3,float m4){
  String result = "<0,"+ String(m1)+ "," + String(m2) + "," + "1," + String(m3) + "," + String(m4) + ">";

  Serial.println(result);
}

void motors_control(float x, float y, float angular) {
  float vel_LD = x + y + angular;
  float vel_RD = x - y - angular;
  float vel_LT = x - y + angular;
  float vel_RT = x + y - angular;

  send_power(vel_RD, vel_RT, -vel_LD, -vel_LT);
  //send_power(w1,w4,w2,w3,angular);
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

/*
  Dribbler.setPeriodHertz(FREQUENCIA_DRIBBLER);
  Dribbler.attach(DRIBBLER_PIN, MIN_THROTTLE_DRIBBLER, MAX_THROTTLE_DRIBBLER);
*/

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }
  peer.channel = 0;  
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_address_feedback, 6);
  if (esp_now_add_peer(&peer) != ESP_OK){
    Serial.println("Failed to add peer");
    ESP.restart();
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);



  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);

 // DataReceived.data_received = 0;
 // DataFeedback.Data_feedback = 0;
}

int readBattery(){
  int voltage = analogRead(VOLTAGE_SENSOR_PIN);
  return voltage;
}

void loop() {

  /*
  sensor_kick = digitalRead(SENSOR_KICKER);
  DataFeedback.sensor_kick = sensor_kick;
  DataFeedback.battery = readBattery();
  DataFeedback.rssi = rssi;
  strcpy(tempChars, commands); // necessário para proteger a informação original
  //Serial.println(tempChars);
  parseData();

  second_mark = millis();
  //Serial.println(second_mark - first_mark);
  if (second_mark - first_mark > 500) {
    v_l = 0.00;
    v_a = 0.00;
    theta = 0.00;
    last_error = 0;
    error_sum = 0;
    stop = true;

  }/*
  if (sensor_kick == 1){
    ledcWrite(kick_straight); //ver como vao ficar os parametros da solenoide
  }
  if (dribbler == 1){
    Dribbler.write(MAX_DRIBBLER);
  }
  motors_control(v_l, v_a,theta); //aplica os valores para os motores*/
  if(status_com == 1){
    DataFeedback.password = FB_PASSWORD;
    DataFeedback.rssi = rssi;
    DataFeedback.id = 1;
    DataFeedback.battery = 250;
    esp_err_t result = esp_now_send(mac_address_feedback, (uint8_t *) &DataFeedback, sizeof(DataFeedback));
  }
 /* else{
    Serial.println("Nenhuma mensagem recebida");
  }*/

}



/*
void parseData(){
    char * strtokIndx;
  
    strtokIndx = strtok(tempChars, ",");
    
    while (strtokIndx != NULL){
        id = atoi(strtokIndx);
        
        if(id == robot_id){         
          strtokIndx = strtok(NULL, ",");  
          v_a = atof(strtokIndx);       
          strtokIndx = strtok(NULL, ",");         
          v_l = atof(strtokIndx);
          strtokIndx = strtok(NULL, ",");
          kick_straight = atof(strtokIndx);
          strtokIndx = strtok(NULL, ","); 
          dribbler = strtok(NULL, ",");
          strtokIndx = strtok(NULL, ",");
          //kick_dug = atof(strtokIndx);
          //strtokIndx = strtok(NULL, ","); 
          theta = atof(strtokIndx);
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
*/