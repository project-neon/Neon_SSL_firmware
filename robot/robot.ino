#define VOLTAGE_SENSOR_PIN 34
#define SENSOR_KICKER 14
#define KICKER_PIN 18
#define DRIBBLER_PIN 32
#define MAX_DRIBBLER 110 //não é definitivo

#define FREQUENCIA_DRIBBLER 50
#define MIN_THROTTLE_DRIBBLER 1048
#define MAX_THROTTLE_DRIBBER 1952

#define ROBOT_PASSWORD 2400
#define FB_PASSWORD 1500

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
//#include <ESP32Servo.h>


uint8_t mac_address_feedback[6] = {0x08, 0xB6, 0x1F, 0x28, 0xE3, 0x94}; //mac address do feedback

uint8_t mac_address_station[6] = {0xC0, 0x49, 0xEF, 0xE4, 0xDC, 0xE4};
//C0:49:EF:E4:DC:E4

float L = 0.0785; //distancia entre roda e centro do robo
float r = 0.03;

int robot_id = 2;
int id;
int first_mark = 0, second_mark;
int last = 0;

float vel_step = 100;
float a = 0; //aceleração
int last_time = 0;
float dt = 0;
float previous_RD = 0;
float previous_RT = 0;
float previous_LD = 0;
float previous_LT = 0;

bool stop=0;
volatile bool new_data=0;

float v_l, v_a, th;
int last_error = 0;
float error_sum = 0;

int32_t rssi = 0;

const byte numChars = 64;
char commands[numChars];
char tempChars[numChars];
char last_message[numChars];

esp_now_peer_info_t peer;

//struct received
typedef struct struct_data {
  int password;
  char message[numChars];
} struct_data;

//struct to send

/*
- feedback password
- id
- rssi;
- battery
*/

typedef struct struct_feedback {
  int password;
  int id;
  int rssi;
  float battery; 
} struct_feedback;


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
  memcpy(&DataReceived, incomingData, sizeof(DataReceived));

  if (DataReceived.password != ROBOT_PASSWORD) return;

  strcpy(commands, DataReceived.message);
  new_data=1;
}

void promiscuous_rx_cb(void *buff, wifi_promiscuous_pkt_type_t type) {
  /*
  rssi < -90 dbm : muito ruim
  rssi ~ -65 dbm: sinal ok
  rssi > -55 dbm: sinal bom
  rssi > -30 dbm: sinal ótimo
  */
  if (type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  bool is_from_station = true;

  for (int i = 0; i < 6; i++) {
      if (hdr->addr2[i] != mac_address_station[i]) {
          is_from_station = false;
          break;
      }
  }
  if (is_from_station)  rssi = ppkt->rx_ctrl.rssi;
}


void send_power(float m1,float m2,float m3,float m4){
  String result = "<0,"+ String(m1)+ "," + String(m2) + "," + "1," + String(m3) + "," + String(m4) + ">";

  Serial.println(result);
}

float calculate_motor(float v_x, float v_y, float angular, float L,float radius, int motor){
  float vel = 0;
  if (motor == 1) {
    vel = ((2*L*angular) - (sqrt(3)*v_x) + (sqrt(3)*v_y))/(2*r);
  }
  if (motor == 2) {
    vel = ((2*L*angular) - (sqrt(3)*v_x) - (sqrt(3)*v_y))/(2*r);
  }
  if (motor == 3) {
    vel = ((2*L*angular) + (sqrt(3)*v_x) - (sqrt(3)*v_y))/(2*r);
  }  
  if (motor == 4) {
    vel = ((2*L*angular) + (sqrt(3)*v_x) + (sqrt(3)*v_y))/(2*r);
  }
  return vel;

}

void motors_control(float x, float y, float angular) {
  float vel_RD = calculate_motor(x, y, angular, L, r, 1);
  float vel_RT = calculate_motor(x, y, angular, L, r, 2);
  float vel_LD = calculate_motor(x, y, angular, L, r, 3);
  float vel_LT = calculate_motor(x, y, angular, L, r, 4);

  vel_RD = acceleration(vel_RD, previous_RD, vel_step);
  vel_RT = acceleration(vel_RT, previous_RT, vel_step);
  vel_LD = acceleration(vel_LD, previous_LD, vel_step);
  vel_LT = acceleration(vel_LT, previous_LT, vel_step);  

  send_power(vel_RD, vel_RT, vel_LD, vel_LT);

  previous_RD = vel_RD;
  previous_RT = vel_RT;
  previous_LD = vel_LD;
  previous_LT = vel_LT;
}


float acceleration(float m, float previous_m, float vel_step){
  a = (abs(m-previous_m))/dt;
  
  if(a > (vel_step)){
    if(m > previous_m){
      m = previous_m + (vel_step * dt);
      }
      
    else{
      m = previous_m - (vel_step * dt);
      }
  }
  return m;
}


void setup() {
  delay(100);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(VOLTAGE_SENSOR_PIN, OUTPUT);
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
  esp_now_register_recv_cb(OnDataRecv);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}

float readBattery(){
  int adc_read = analogRead(VOLTAGE_SENSOR_PIN);
 
  float voltage_output_read = (adc_read*0.00511);

  return voltage_output_read;
}

void loop() {
  strcpy(tempChars, commands); // necessário para proteger a informação original

  if(new_data) parseData(); //envia os dados para o array commands

  second_mark = millis();
  //Serial.println(second_mark - first_mark);
  if (second_mark - first_mark > 300) {
    v_l = 0.00;
    v_a = 0.00;
    th = 0.00;
    last_error = 0;
    error_sum = 0;
    stop = true;
  }
  /*
  if (sensor_kick == 1){
    ledcWrite(kick_straight); //ver como vao ficar os parametros da solenoide
  }
  if (dribbler == 1){
    Dribbler.write(MAX_DRIBBLER);
  }*/
  
  int crt = millis();
  dt = (crt - last_time)/1000.0;
  last_time = crt;
  //if (!stop) 
  motors_control(v_l, v_a, th); //aplica os valores para os motores

  if(new_data){
    DataFeedback.password = FB_PASSWORD;
    DataFeedback.rssi = rssi;
    DataFeedback.id = robot_id;
    DataFeedback.battery = readBattery();
    esp_err_t result = esp_now_send(mac_address_feedback, (uint8_t *) &DataFeedback, sizeof(DataFeedback));
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
          first_mark = millis();        
          strtokIndx = strtok(NULL, ",");  
          v_l = atof(strtokIndx);       
          strtokIndx = strtok(NULL, ",");         
          v_a = atof(strtokIndx);
          strtokIndx = strtok(NULL, ",");         
          th = atof(strtokIndx);
          strtokIndx = strtok(NULL, ","); 
        }

       else{
          strtokIndx = strtok(NULL, ",");     
          strtokIndx = strtok(NULL, ",");         
          strtokIndx = strtok(NULL, ",");         
          strtokIndx = strtok(NULL, ","); 
        }
    }   
}
