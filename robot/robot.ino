#include "config.h"
#include "sensor.h"
#include "communication.h"
#include "skills.h"
#include "speed_control.h"


void setup(){
    Serial.begin(115200);

    delay(100);

    WiFi.mode(WIFI_STA);
    pinMode(VOLTAGE_SENSOR_PIN, OUTPUT);
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
    esp_wifi_set_promiscuous(useFeedback);
    if (useFeedback) esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}

void loop(){
    strcpy(tempChars, commands);

    if(new_data) parseData();
    second_mark = millis();

    if (second_mark - first_mark > FAILSAFE_MS) failSafe();
    if ((kick_time != 0) && (second_mark - kicker_mark > KICK_COOLDOWN_MS)) kick(kick_time);

    crt = millis();
    dt = (crt - last_time)/1000.0;
    last_time = crt;

    if (!stop) motors_control(v_l, v_a, th);
    if((new_data) && (useFeedback)) sendFeedback();
}