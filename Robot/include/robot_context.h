#pragma once

#include <cstdint>
#include <cstring>
#include <esp_now.h>
#include "config.h"
#include "types.h"

class RobotContext {
public:
    int robot_id;
    bool use_feedback;
    bool compute_rssi;
    uint8_t mac_feedback[6];
    uint8_t mac_station[6];
    float L;
    float r;
    float vel_step;

    char commands[MESSAGE_LENGTH_BYTES] = {};
    char temp_chars[MESSAGE_LENGTH_BYTES] = {};
    char last_message[MESSAGE_LENGTH_BYTES] = {};

    int id = 0;
    bool new_data = false;
    bool stop = false;
    float v_l = 0.0f;
    float v_a = 0.0f;
    float th = 0.0f;
    int kick_time = 0;
    bool waiting_to_kick = false;

    int first_mark = 0;
    int second_mark = 0;
    int kicker_mark = 0;
    int charge_kicker = 0;
    int crt = 0;
    float dt = 0.0f;
    int last_time = 0;

    uint32_t last_feedback_ms = 0;
    uint32_t last_probe_ms = 0;
    uint32_t pause_until_ms = 0;
    bool paused = false;
    int fail_streak = 0;

    struct_data          data_received = {};
    struct_feedback      data_feedback = {};
    esp_now_peer_info_t  peer = {};
    esp_now_send_status_t last_send_status = ESP_NOW_SEND_FAIL;

    bool charge_enabled = false;
    unsigned long charge_on_since_ms = 0;
    unsigned long can_kick_since = 0;
    unsigned long kick_done_ms = 0;
    bool waiting_before_recharge = false;

    float prev_rd = 0.0f;
    float prev_rt = 0.0f;
    float prev_ld = 0.0f;
    float prev_lt = 0.0f;

    int last_error = 0;
    float error_sum = 0.0f;

    int32_t rssi = 0;

    RobotContext(int id,
                 bool fb,
                 bool rssi_en,
                 const uint8_t mac_fb[6],
                 const uint8_t mac_st[6],
                 float wheelbase,
                 float radius,
                 float step)
        : robot_id(id)
        , use_feedback(fb)
        , compute_rssi(rssi_en)
        , L(wheelbase)
        , r(radius)
        , vel_step(step)
    {
        memcpy(mac_feedback, mac_fb, 6);
        memcpy(mac_station, mac_st, 6);
    }
};

extern RobotContext* robot;
