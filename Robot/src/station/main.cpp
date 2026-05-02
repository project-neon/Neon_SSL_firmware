#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// --- Constants and configuration ---
constexpr uint8_t BROADCAST_ADDRESS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
constexpr int LED_PIN = 2;
constexpr size_t MESSAGE_LENGTH = 200;
constexpr char MSG_START_MARKER = '<';
constexpr char MSG_END_MARKER = '>';
constexpr bool AUTO_MODE_ENABLED = false;
constexpr int AUTO_MODE_SEND_INTERVAL_IN_MS = 200;
constexpr int ROBOT_PASSWORD = 2400;
const char DEFAULT_MESSAGE[] = "2,0.5,1.0,0.0,300";

// --- Data Structures ---
struct message_t
{
    int password;
    char message[MESSAGE_LENGTH];
};

bool recv_with_message_markers(char *buffer, size_t buf_len);
void send_message(const message_t &msg);
void print_message(char *msg);

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    pinMode(LED_PIN, OUTPUT);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        ESP.restart();
    }
    else
    {
        Serial.println("ESPNOW OK");
    }

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, BROADCAST_ADDRESS, 6);
    peer.channel = 0;
    peer.encrypt = false;

    if (esp_now_add_peer(&peer) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        ESP.restart();
    }
}

void loop()
{
    static message_t message{};
    static uint32_t last_msg_timestamp_in_ms = 0;
    static char message_buffer[MESSAGE_LENGTH];

    if (AUTO_MODE_ENABLED)
    {
        uint32_t now = millis();
        if (now - last_msg_timestamp_in_ms >= AUTO_MODE_SEND_INTERVAL_IN_MS)
        {
            strncpy(message.message, DEFAULT_MESSAGE, MESSAGE_LENGTH - 1);
            message.message[MESSAGE_LENGTH - 1] = '\0';
            message.password = ROBOT_PASSWORD;
            send_message(message);
            last_msg_timestamp_in_ms = now;
        }
        return;
    }

    if (recv_with_message_markers(message_buffer, MESSAGE_LENGTH))
    {
        strncpy(message.message, message_buffer, MESSAGE_LENGTH - 1);
        message.message[MESSAGE_LENGTH - 1] = '\0';
        message.password = ROBOT_PASSWORD;
        send_message(message);
    }
}

// Encapsulate Serial receiving logic
bool recv_with_message_markers(char *buffer, size_t buf_len)
{
    static bool recv_in_progress = false;
    static size_t ndx = 0;
    bool new_message_received = false;

    while (Serial.available() > 0 && !new_message_received)
    {
        char c = Serial.read();
        if (recv_in_progress)
        {
            if (c != MSG_END_MARKER)
            {
                if (ndx < buf_len - 1)
                {
                    buffer[ndx++] = c;
                }
            }
            else
            {
                buffer[ndx] = '\0';
                recv_in_progress = false;
                ndx = 0;
                new_message_received = true;
            }
        }
        else if (c == MSG_START_MARKER)
        {
            recv_in_progress = true;
            ndx = 0;
        }
    }

    return new_message_received;
}

void send_message(const message_t &msg)
{
    esp_err_t err = esp_now_send(BROADCAST_ADDRESS, (uint8_t *)&msg, sizeof(msg));

    // Pulse the LED to indicate sending
    digitalWrite(LED_PIN, HIGH);
    delay(3);
    digitalWrite(LED_PIN, LOW);
}