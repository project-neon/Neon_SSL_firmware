#include <WiFi.h>
#include "robot_context.h"
#include "communication.h"
#include "kicker.h"
#include "sensor.h"
#include "speed_control.h"

RobotContext* robot = nullptr;

void setup()
{
    const uint8_t mac_st[6] = {0xA4, 0xCF, 0x12, 0x72, 0xB7, 0x20};
    const uint8_t mac_fb[6] = {0x08, 0xB6, 0x1F, 0x28, 0xE3, 0x94};
    robot = new RobotContext(2, true, true, mac_fb, mac_st, 0.0785f, 0.03f, 100.0f);

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    pinMode(2, OUTPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);

    setup_kicker();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        ESP.restart();
        return;
    }

    robot->peer.channel = 0;
    robot->peer.encrypt = false;
    memcpy(robot->peer.peer_addr, robot->mac_feedback, 6);
    if (esp_now_add_peer(&robot->peer) != ESP_OK) {
        Serial.println("Failed to add peer");
        ESP.restart();
    }

    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

    esp_wifi_set_promiscuous(robot->compute_rssi);
    if (robot->compute_rssi) {
        esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
    }
}

void loop()
{
    strcpy(robot->scratch_buffer, robot->command_buffer);

    bool incoming = robot->new_data;
    if (incoming) {
        parse_data();
        robot->new_data = false;
    }

    int now_ms = millis();
    if (now_ms - robot->last_command_ms > FAILSAFE_MS) {
        fail_safe();
    }

    kicker_control();

    int crt = millis();
    robot->dt = (crt - robot->last_time) / 1000.0f;
    robot->last_time = crt;

    if (!robot->stop) {
        motors_control();
    }

    handle_feedback();
}
