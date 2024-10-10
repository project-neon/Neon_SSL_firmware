#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

// This is de code for the board that is in robots
int robot_id = 0;
int id;
int first_mark = 0, second_mark;
int last = 0;

bool stop=0;
volatile bool new_data=0;

float v_l, v_a, th;
float last_error = 0;
float error_sum = 0;

const byte numChars = 64;
char commands[numChars];
char tempChars[numChars];
char last_message[numChars];

typedef struct struct_message{
  char message[64];
  } struct_message;

struct_message rcv_commands;

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&rcv_commands, incomingData, sizeof(rcv_commands));
  // Update the structures with the new incoming data
  strcpy(commands, rcv_commands.message);
  new_data=1;
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
  delay(100);
  Serial.begin(115200);
  // configurações comunicação
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_start());
  ESP_ERROR_CHECK( esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  //Serial.print("ESP Board MAC Address:  ");
  //Serial.println(WiFi.macAddress());
  // configuração mpu
  //mpu_init();
  delay(100);
}

void loop() {
  
  strcpy(tempChars, commands); // necessário para proteger a informação original
  //Serial.println(tempChars);
  if(new_data) parseData();

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

//  if(millis()-last > 1){
//    last = millis();
  if(!stop) motors_control(v_l, v_a,th); //aplica os valores para os motores
//  } 
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
        }

       else{
          strtokIndx = strtok(NULL, ",");     
          strtokIndx = strtok(NULL, ",");         
          strtokIndx = strtok(NULL, ","); 
        }
    } 
   

  
   
}
