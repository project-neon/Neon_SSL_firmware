#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"


#define KICK_PIN    33
#define CHARGE_PIN  18

// regras de segurança
const unsigned long KICK_COOLDOWN_MS   = 2000; // entre chutes
const unsigned long TIME_AFTER_CHARGE  = 500;   // ms após desligar carga antes do chute
const unsigned long TIME_BEFORE_CHARGE = 10; // ms após desligar carga antes do chute
const unsigned long MIN_CHARGE_TIME_MS = 2000; // ms de carga antes de poder chutar


static bool charge_enabled = false;
static unsigned long charge_on_since_ms = 0;
static unsigned long last_kick_ms = 0;

esp_now_peer_info_t peer;

//struct received
typedef struct struct_data {
  int time_us;
} struct_data;


struct_data DataReceived;

void charge_on() {
  if (!charge_enabled) {
    digitalWrite(CHARGE_PIN, HIGH);
    charge_enabled = true;
    charge_on_since_ms = millis();
    Serial.println(F("CHARGE ON"));
  }
}

void charge_off() {
  if (charge_enabled) {
    digitalWrite(CHARGE_PIN, LOW);
    charge_enabled = false;
    Serial.println(F("CHARGE OFF"));
  }
}

bool can_kick_now() {
  unsigned long now = millis();

  if (!charge_enabled) {
    Serial.println(F("[BLOCK] Carregamento OFF"));
    return false;
  }
  if (now - charge_on_since_ms < MIN_CHARGE_TIME_MS) {
    Serial.print(F("[BLOCK] Carregando ha "));
    Serial.print(now - charge_on_since_ms);
    Serial.println(F(" ms (< 2000 ms)"));
    return false;
  }
  if (now - last_kick_ms < KICK_COOLDOWN_MS) {
    Serial.print(F("[BLOCK] Cooldown restante: "));
    Serial.print(KICK_COOLDOWN_MS - (now - last_kick_ms));
    Serial.println(F(" ms"));
    return false;
  }
  return true;
}

uint32_t calc_power(int pot){
  return (uint32_t) max(0, min(20000, (int) map(pot, 0, 9, 0, 20000)));
}

bool do_kick(uint32_t pulse_us) {
  if (!can_kick_now()) return false;

  charge_off();
  delay(TIME_AFTER_CHARGE);
  Serial.print(F("[KICK] Pulso de "));
  Serial.print(pulse_us);
  Serial.println(F(" us"));
  digitalWrite(KICK_PIN, HIGH);
  delayMicroseconds(pulse_us);
  digitalWrite(KICK_PIN, LOW);

  last_kick_ms = millis();

  charge_on();

  return true;
}

void OnDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
  digitalWrite(2,HIGH);
  delay(3);
  digitalWrite(2,LOW);

  memcpy(&DataReceived, incomingData, sizeof(DataReceived));

  int time = DataReceived.time_us;
}


void setup() {
  Serial.begin(115200);
  
  pinMode(KICK_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(KICK_PIN, LOW);
  digitalWrite(CHARGE_PIN, LOW);
  charge_on();

  last_kick_ms = millis() - KICK_COOLDOWN_MS;


  delay(100);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  int time = DataReceived.time_us;
  if(time > 0) {
    do_kick(calc_power(time));
  }
  DataReceived.time_us = 0;
  delay(1); 
}
