#include <WiFi.h> //esp1 COM3 
#include "config.h"
#include "sensor.h"
#include "communication.h"
#include "skills.h"
#include "kicker.h"
#include "speed_control.h"

// // --- benchmarking do loop ---
// static uint32_t loop_start_us = 0;
// static uint32_t acc_us = 0;
// static uint32_t n_iters = 0;
// static uint32_t max_us = 0, min_us = 0xFFFFFFFF;
// static uint32_t last_print_ms = 0;

// inline void bench_loop_begin() {
//   loop_start_us = micros();
// }

// inline void bench_loop_end() {
//   uint32_t dt = micros() - loop_start_us;
//   acc_us += dt;
//   n_iters++;
//   if (dt > max_us) max_us = dt;
//   if (dt < min_us) min_us = dt;

//   uint32_t now_ms = millis();
//   if (now_ms - last_print_ms >= 1000) { // imprime a cada ~1s
//     float avg_us = (n_iters > 0) ? (float)acc_us / (float)n_iters : 0.0f;
//     Serial.print("[LOOP] avg_us=");
//     Serial.print(avg_us, 1);
//     Serial.print(" | min_us=");
//     Serial.print(min_us);
//     Serial.print(" | max_us=");
//     Serial.print(max_us);
//     Serial.print(" | iters=");
//     Serial.print(n_iters);
//     Serial.print(" | useFeedback=");
//     Serial.print(useFeedback ? "1" : "0");   // vem de config.h
//     Serial.print(" | computeRSSI=");
//     Serial.println(computeRSSI ? "1" : "0"); // vem de config.h
//     // zera janelas
//     acc_us = 0; n_iters = 0; max_us = 0; min_us = 0xFFFFFFFF;
//     last_print_ms = now_ms;
//   }
// }


void setup(){
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
    peer.channel = 0;  
    peer.encrypt = false;
    memcpy(peer.peer_addr, mac_address_feedback, 6);
    if (esp_now_add_peer(&peer) != ESP_OK){
        Serial.println("Failed to add peer");
        ESP.restart();
    }
    esp_now_register_recv_cb(OnDataRecv);
    esp_wifi_set_promiscuous(useFeedback);
    if (computeRSSI) esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}

void loop(){
   // bench_loop_begin();
    strcpy(tempChars, commands);
    if(new_data) parseData();
    second_mark = millis();
    if (second_mark - first_mark > FAILSAFE_MS) failSafe();
    //if ((kick_time != 0) && (second_mark - kicker_mark > KICK_COOLDOWN_MS)) kicker_control();
    kicker_control();
    crt = millis();
    dt = (crt - last_time)/1000.0;
    last_time = crt;
    if (!stop) motors_control(v_l, v_a, th);
    if((new_data) && (useFeedback)) sendFeedback();
   // bench_loop_end();
}
