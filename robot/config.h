int robot_id = 2;
bool useFeedback = true;


#define VOLTAGE_SENSOR_PIN 34
#define SENSOR_KICKER 14
#define KICKER_PIN 32
#define DRIBBLER_PIN 32
#define MAX_DRIBBLER 110 //não é definitivo

#define FREQUENCIA_DRIBBLER 50
#define MIN_THROTTLE_DRIBBLER 1048
#define MAX_THROTTLE_DRIBBER 1952

#define ROBOT_PASSWORD 2400
#define FB_PASSWORD 1500

uint8_t mac_address_feedback[6] = {0x08, 0xB6, 0x1F, 0x28, 0xE3, 0x94};
uint8_t mac_address_station[6] = {0xC0, 0x49, 0xEF, 0xE4, 0xDC, 0xE4};



float L = 0.0785;
float r = 0.03;

const byte numChars = 200;

const unsigned long KICK_COOLDOWN_MS = 500;
const unsigned long FAILSAFE_MS = 300;

float vel_step = 100;


//----------------------------------------//
float v_l, v_a, th;
int first_mark = 0, second_mark, kicker_mark;
char commands[numChars];
char tempChars[numChars];
char last_message[numChars];
int id;
bool new_data = false;
int last_error = 0;
float error_sum = 0;
bool stop = false;
float dt = 0;
int last_time = 0;
int kick_time = 0;

int crt;

float a = 0;
//----------------------------------------//

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

