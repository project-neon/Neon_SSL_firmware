#include <Arduino.h>

//Comandos:
//  C1           -> ligar carregamento
//  C0           -> desligar carregamento
//  K <micros>   -> chutar (exemplo: "K 800")

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
  delayMicroseconds(TIME_BEFORE_CHARGE);

  last_kick_ms = millis();

  charge_on();

  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(KICK_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(KICK_PIN, LOW);
  digitalWrite(CHARGE_PIN, LOW);
  charge_on();

  last_kick_ms = millis() - KICK_COOLDOWN_MS;
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("C1")) {
      charge_on();
    } else if (cmd.equalsIgnoreCase("C0")) {
      charge_off();
    } else if (cmd.startsWith("K")) {
      int sp = cmd.indexOf(' ');
      if (sp > 0) {
        long pulse_us = cmd.substring(sp + 1).toInt();
        if (pulse_us > 0) {
          if (!do_kick((uint32_t)pulse_us)) {
            Serial.println(F("KICK Abortado pelas regras (carga/tempo/cooldown)"));
          }
        } else {
          Serial.println(F("Use: K <micros> com valor > 0"));
        }
      } else {
        Serial.println(F("Formato: K <micros>"));
      }
    } else if (cmd.length()) {
      Serial.println(F("Comando invalido"));
    }
  }

  delay(1);
}
