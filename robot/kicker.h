#define KICK_PIN    33
#define CHARGE_PIN  18

// regras de segurança
//const unsigned long KICK_COOLDOWN_MS   = 2000; // entre chutes
const unsigned long TIME_AFTER_CHARGE  = 500;   // ms após desligar carga antes do chute
const unsigned long TIME_BEFORE_CHARGE = 10; // ms após desligar carga antes do chute
const unsigned long MIN_CHARGE_TIME_MS = 2000; // ms de carga antes de poder chutar


static bool charge_enabled = false;
static unsigned long charge_on_since_ms = 0;
//static unsigned long last_kick_ms = 0;
static unsigned long can_kick_since = 0;
static unsigned long kick_done_ms = 0;   

static bool waiting_before_recharge = false;


void charge_on() {
  if (!charge_enabled) {
    digitalWrite(CHARGE_PIN, HIGH);
    charge_enabled = true;
    charge_on_since_ms = millis();
//    Serial.println(F("CHARGE ON"));
  }
}

void charge_off() {
  if (charge_enabled) {
    digitalWrite(CHARGE_PIN, LOW);
    charge_enabled = false;
//    Serial.println(F("CHARGE OFF"));
  }
}


bool can_kick_now() {
  unsigned long now = millis();
  if (!charge_enabled) return false;
  if (now - charge_on_since_ms < MIN_CHARGE_TIME_MS) return false;
  return true; 
}

uint32_t calc_power(int pot){
  return (uint32_t) max(0, min(20000, (int) map(pot, 0, 9, 0, 20000)));
}

bool do_kick(uint32_t pulse_us) {
  digitalWrite(KICK_PIN, HIGH);
  delayMicroseconds(pulse_us);
  digitalWrite(KICK_PIN, LOW);
  return true;
}

void setup_kicker() {
//  Serial.begin(115200);
  
  pinMode(KICK_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  digitalWrite(KICK_PIN, LOW);
  digitalWrite(CHARGE_PIN, LOW);
  charge_on();

}




void kicker_control(){
  unsigned long now = millis();
  if (waiting_before_recharge){
    if (now - kick_done_ms >= TIME_BEFORE_CHARGE) {
      if (!charge_enabled) charge_on();
      waiting_before_recharge = false;
    }
    return;
  }

  if (kick_time <= 0) {
    if (!charge_enabled) charge_on();
    waiting_to_kick = false;
    return;
  }

  if (!waiting_to_kick) {
    if (can_kick_now()) {
      charge_off();                 
      can_kick_since = millis();  
      waiting_to_kick = true;
    }
    return; 
  }
  if (now - can_kick_since >= TIME_AFTER_CHARGE) {
    if (do_kick(calc_power(kick_time))) {
      kick_time = 0;
      waiting_to_kick = false;
      waiting_before_recharge = true;
      kick_done_ms = now;
      //if (!charge_enabled) charge_on();                  
   } 
  }
}