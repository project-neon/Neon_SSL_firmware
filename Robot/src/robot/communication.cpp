#include <Arduino.h>
#include "communication.h"
#include "sensor.h"

void OnDataSent(const uint8_t *mac, esp_now_send_status_t status)
{
    robot->last_send_status = status;
    if (status == ESP_NOW_SEND_SUCCESS) {
        robot->fail_streak = 0;
    } else if (robot->fail_streak < 255) {
        robot->fail_streak++;
    }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&robot->data_received, incomingData, sizeof(robot->data_received));
    if (robot->data_received.password != ROBOT_PASSWORD) {
        return;
    }
    robot->last_command_ms = millis();
    strcpy(robot->command_buffer, robot->data_received.message);
    robot->new_data = true;
}

bool send_feedback()
{
    robot->data_feedback.password = FEEDBACK_PASSWORD;
    robot->data_feedback.rssi     = robot->rssi;
    robot->data_feedback.id       = robot->robot_id;
    robot->data_feedback.battery  = read_battery();

    esp_err_t e = esp_now_send(robot->mac_feedback,
                               (uint8_t *)&robot->data_feedback,
                               sizeof(robot->data_feedback));
    if (e != ESP_OK) {
        if (robot->fail_streak < 255) {
            robot->fail_streak++;
        }
        return false;
    }
    return true;
}

void handle_feedback()
{
    if (!robot->use_feedback) {
        robot->paused      = false;
        robot->fail_streak = 0;
        return;
    }

    const uint32_t now = millis();

    if (!robot->paused && robot->fail_streak >= MAX_FAILS_BEFORE_PAUSE) {
        robot->paused         = true;
        robot->pause_until_ms = now + PAUSE_COOLDOWN_MS;
    }

    if (!robot->paused) {
        if (now - robot->last_feedback_ms >= FEEDBACK_INTERVAL_MS) {
            if (send_feedback()) {
                robot->last_feedback_ms = now;
            }
        }
        return;
    }

    if (now >= robot->pause_until_ms) {
        if (now - robot->last_probe_ms >= PROBE_PERIOD_MS) {
            if (send_feedback()) {
                robot->last_probe_ms = now;
            }
            if (robot->last_send_status == ESP_NOW_SEND_SUCCESS) {
                robot->paused           = false;
                robot->fail_streak      = 0;
                robot->last_feedback_ms = now;
            }
        }
    }
}

void parse_data()
{
    char *strtokIndx;
    strtokIndx = strtok(robot->scratch_buffer, ",");

    while (strtokIndx != NULL) {
        robot->parsed_id = atoi(strtokIndx);

        if (robot->parsed_id == robot->robot_id) {
            robot->stop = false;

            strtokIndx = strtok(NULL, ",");
            robot->v_linear = atof(strtokIndx);
            strtokIndx = strtok(NULL, ",");
            robot->v_angular = atof(strtokIndx);
            strtokIndx = strtok(NULL, ",");
            robot->throttle = atof(strtokIndx);
            strtokIndx = strtok(NULL, ",");
            if (!robot->waiting_to_kick) {
                robot->kick_time = atof(strtokIndx);
            }
            strtokIndx = strtok(NULL, ",");
        } else {
            strtokIndx = strtok(NULL, ",");
            strtokIndx = strtok(NULL, ",");
            strtokIndx = strtok(NULL, ",");
            strtokIndx = strtok(NULL, ",");
            strtokIndx = strtok(NULL, ",");
        }
    }
}
