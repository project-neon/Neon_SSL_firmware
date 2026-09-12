#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <types.h>

// --- Constants and configuration ---
constexpr uint8_t BROADCAST_ADDRESS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
constexpr int LED_PIN = 2;
constexpr char MSG_START_MARKER = '<';
constexpr char MSG_END_MARKER = '>';
constexpr bool AUTO_MODE_ENABLED = false;
constexpr int AUTO_MODE_SEND_INTERVAL_IN_MS = 200;
const char DEFAULT_MESSAGE[] = "2,0.5,1.0,0.0,300";

bool recv_with_message_markers(char *buffer, size_t buf_len);
void send_command(const robot_command &msg);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(LED_PIN, OUTPUT);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();
  } else {
    Serial.println("ESPNOW OK");
  }

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, BROADCAST_ADDRESS, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    ESP.restart();
  }
}

void loop() {
  static robot_command command{};
  static uint32_t last_msg_timestamp_in_ms = 0;
  static char message_buffer[MESSAGE_LENGTH_BYTES];

  if (AUTO_MODE_ENABLED) {
    uint32_t now = millis();
    if (now - last_msg_timestamp_in_ms >= AUTO_MODE_SEND_INTERVAL_IN_MS) {
      strncpy(command.message, DEFAULT_MESSAGE, MESSAGE_LENGTH_BYTES - 1);
      command.message[MESSAGE_LENGTH_BYTES - 1] = '\0';
      command.password = ROBOT_PASSWORD;
      send_command(command);
      last_msg_timestamp_in_ms = now;
    }
    return;
  }

  if (recv_with_message_markers(message_buffer, MESSAGE_LENGTH_BYTES)) {
    strncpy(command.message, message_buffer, MESSAGE_LENGTH_BYTES - 1);
    command.message[MESSAGE_LENGTH_BYTES - 1] = '\0';
    command.password = ROBOT_PASSWORD;
    send_command(command);
  }
}

// Encapsulate Serial receiving logic
bool recv_with_message_markers(char *buffer, size_t buf_len) {
  static bool recv_in_progress = false;
  static size_t ndx = 0;
  bool new_message_received = false;

  while (Serial.available() > 0 && !new_message_received) {
    char c = Serial.read();
    if (recv_in_progress) {
      if (c != MSG_END_MARKER) {
        if (ndx < buf_len - 1) {
          buffer[ndx++] = c;
        }
      } else {
        buffer[ndx] = '\0';
        recv_in_progress = false;
        ndx = 0;
        new_message_received = true;
      }
    } else if (c == MSG_START_MARKER) {
      recv_in_progress = true;
      ndx = 0;
    }
  }

  return new_message_received;
}

void send_command(const robot_command &msg) {
  esp_err_t err = esp_now_send(BROADCAST_ADDRESS, (uint8_t *)&msg, sizeof(msg));

  // Pulse the LED to indicate sending
  digitalWrite(LED_PIN, HIGH);
  delay(3);
  digitalWrite(LED_PIN, LOW);
}
