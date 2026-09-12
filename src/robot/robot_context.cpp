#include "robot_context.h"

#include <Arduino.h>

#include <cmath>

#include "sensor.h"

RobotContext::RobotContext(int id, bool fb, bool rssi_en,
                           const uint8_t mac_fb[6], const uint8_t mac_st[6],
                           float wbase, float wradius, float step)
    : robot_id_(id),
      use_feedback_(fb),
      compute_rssi_(rssi_en),
      wheelbase_(wbase),
      wheel_radius_(wradius),
      vel_step_(step) {
  memcpy(mac_feedback_, mac_fb, 6);
  memcpy(mac_station_, mac_st, 6);
}

void RobotContext::setup_kicker_pins() {
  pinMode(KICKER_PIN, OUTPUT);
  pinMode(CHARGE_KICKER_PIN, OUTPUT);
  digitalWrite(KICKER_PIN, LOW);
  digitalWrite(CHARGE_KICKER_PIN, LOW);
}

void RobotContext::setup_peer(uint8_t channel) {
  peer_.channel = channel;
  peer_.encrypt = false;
  memcpy(peer_.peer_addr, mac_feedback_, 6);
}

void RobotContext::handle_received_packet(const uint8_t *data, int len) {
  memcpy(&data_received_, data, sizeof(data_received_));
  if (data_received_.password != ROBOT_PASSWORD) {
    return;
  }
  last_command_ms_ = millis();
  strcpy(command_buffer_, data_received_.message);
  new_data_ = true;
}

void RobotContext::handle_send_status(esp_now_send_status_t status) {
  last_send_status_ = status;
  if (status == ESP_NOW_SEND_SUCCESS) {
    fail_streak_ = 0;
  } else if (fail_streak_ < 255) {
    fail_streak_++;
  }
}

void RobotContext::process_command() {
  if (!new_data_) return;

  strcpy(scratch_buffer_, command_buffer_);
  parse_command_internal();
  new_data_ = false;
}

void RobotContext::parse_command_internal() {
  char *tok = strtok(scratch_buffer_, ",");

  while (tok != NULL) {
    parsed_id_ = atoi(tok);

    if (parsed_id_ == robot_id_) {
      stop_ = false;

      tok = strtok(NULL, ",");
      v_linear_ = atof(tok);
      tok = strtok(NULL, ",");
      v_angular_ = atof(tok);
      tok = strtok(NULL, ",");
      throttle_ = atof(tok);
      tok = strtok(NULL, ",");
      if (!waiting_to_kick_) {
        kick_time_ = atof(tok);
      }
      tok = strtok(NULL, ",");
    } else {
      tok = strtok(NULL, ",");
      tok = strtok(NULL, ",");
      tok = strtok(NULL, ",");
      tok = strtok(NULL, ",");
      tok = strtok(NULL, ",");
    }
  }
}

// ══════════════════════════════════════════════════════════════════
// Fail-safe
// ══════════════════════════════════════════════════════════════════

void RobotContext::fail_safe_check() {
  v_linear_ = 0.0f;
  v_angular_ = 0.0f;
  throttle_ = 0.0f;
  last_error_ = 0;
  error_sum_ = 0.0f;
  stop_ = true;

  kick_time_ = 0;
  waiting_to_kick_ = false;
  end_charge();
}

// ══════════════════════════════════════════════════════════════════
// Kicker
// ══════════════════════════════════════════════════════════════════

void RobotContext::begin_charge() {
  if (!charge_enabled_) {
    digitalWrite(CHARGE_KICKER_PIN, HIGH);
    charge_enabled_ = true;
    charge_on_since_ms_ = millis();
  }
}

void RobotContext::end_charge() {
  if (charge_enabled_) {
    digitalWrite(CHARGE_KICKER_PIN, LOW);
    charge_enabled_ = false;
  }
}

bool RobotContext::can_fire_now(unsigned long now) const {
  if (!charge_enabled_) return false;
  if (now - charge_on_since_ms_ < MIN_CHARGE_TIME_MS) return false;
  return true;
}

void RobotContext::do_kick_pulse(uint32_t us) {
  digitalWrite(KICKER_PIN, HIGH);
  delayMicroseconds(us);
  digitalWrite(KICKER_PIN, LOW);
}

uint32_t RobotContext::kick_pulse_us(int power) const {
  return (uint32_t)max(0, min(20000, (int)map(power, 0, 9, 0, 20000)));
}

void RobotContext::kicker_update() {
  unsigned long now = millis();

  if (waiting_before_recharge_) {
    if (now - kick_done_ms_ >= TIME_BEFORE_CHARGE_MS) {
      if (!charge_enabled_) {
        begin_charge();
      }
      waiting_before_recharge_ = false;
    }
    return;
  }

  if (kick_time_ <= 0) {
    if (!charge_enabled_) {
      begin_charge();
    }
    waiting_to_kick_ = false;
    return;
  }

  if (!waiting_to_kick_) {
    if (can_fire_now(now)) {
      end_charge();
      can_kick_since_ = now;
      waiting_to_kick_ = true;
    }
    return;
  }

  if (now - can_kick_since_ >= TIME_AFTER_CHARGE_MS) {
    do_kick_pulse(kick_pulse_us(kick_time_));
    kick_time_ = 0;
    waiting_to_kick_ = false;
    waiting_before_recharge_ = true;
    kick_done_ms_ = now;
  }
}

// ══════════════════════════════════════════════════════════════════
// Timing
// ══════════════════════════════════════════════════════════════════

void RobotContext::update_timing() {
  int now = millis();
  dt_ = (now - last_time_) / 1000.0f;
  last_time_ = now;
}

// ══════════════════════════════════════════════════════════════════
// Motor control
// ══════════════════════════════════════════════════════════════════

float RobotContext::motor_velocity(int motor_idx, float vx, float vy,
                                   float va) const {
  switch (motor_idx) {
    case 1:
      return (2.0f * wheelbase_ * va - sqrt(3.0f) * vx + vy) /
             (2.0f * wheel_radius_);
    case 2:
      return (sqrt(2.0f) * wheelbase_ * va - vx - vy) /
             (sqrt(2.0f) * wheel_radius_);
    case 3:
      return (sqrt(2.0f) * wheelbase_ * va + vx - vy) /
             (sqrt(2.0f) * wheel_radius_);
    case 4:
      return (2.0f * wheelbase_ * va + sqrt(3.0f) * vx + vy) /
             (2.0f * wheel_radius_);
    default:
      return 0.0f;
  }
}

float RobotContext::ramp(float target, float prev, float dt) const {
  float rate = abs(target - prev) / dt;

  if (rate > vel_step_) {
    if (target > prev) {
      return prev + (vel_step_ * dt);
    } else {
      return prev - (vel_step_ * dt);
    }
  }
  return target;
}

void RobotContext::send_powers(float rd, float rt, float ld, float lt) {
  String result = "<0," + String(rd) + "," + String(rt) + "," + "1," +
                  String(ld) + "," + String(lt) + ">";
  Serial.println(result);
}

void RobotContext::motors_update() {
  if (stop_) return;

  float rd = motor_velocity(1, v_linear_, v_angular_, throttle_);
  float rt = motor_velocity(2, v_linear_, v_angular_, throttle_);
  float ld = motor_velocity(3, v_linear_, v_angular_, throttle_);
  float lt = motor_velocity(4, v_linear_, v_angular_, throttle_);

  rd = ramp(rd, prev_rd_, dt_);
  rt = ramp(rt, prev_rt_, dt_);
  ld = ramp(ld, prev_ld_, dt_);
  lt = ramp(lt, prev_lt_, dt_);

  send_powers(rd, rt, ld, lt);

  prev_rd_ = rd;
  prev_rt_ = rt;
  prev_ld_ = ld;
  prev_lt_ = lt;
}

// ══════════════════════════════════════════════════════════════════
// Feedback
// ══════════════════════════════════════════════════════════════════

bool RobotContext::send_feedback_packet() {
  data_feedback_.password = FEEDBACK_PASSWORD;
  data_feedback_.rssi = rssi_;
  data_feedback_.id = robot_id_;
  data_feedback_.battery = read_battery();

  esp_err_t e = esp_now_send(mac_feedback_, (uint8_t *)&data_feedback_,
                             sizeof(data_feedback_));
  if (e != ESP_OK) {
    if (fail_streak_ < 255) {
      fail_streak_++;
    }
    return false;
  }
  return true;
}

void RobotContext::feedback_update() {
  if (!use_feedback_) {
    paused_ = false;
    fail_streak_ = 0;
    return;
  }

  const uint32_t now = millis();

  if (!paused_ && fail_streak_ >= MAX_FAILS_BEFORE_PAUSE) {
    paused_ = true;
    pause_until_ms_ = now + PAUSE_COOLDOWN_MS;
  }

  if (!paused_) {
    if (now - last_feedback_ms_ >= FEEDBACK_INTERVAL_MS) {
      if (send_feedback_packet()) {
        last_feedback_ms_ = now;
      }
    }
    return;
  }

  if (now >= pause_until_ms_) {
    if (now - last_probe_ms_ >= PROBE_PERIOD_MS) {
      if (send_feedback_packet()) {
        last_probe_ms_ = now;
      }
      if (last_send_status_ == ESP_NOW_SEND_SUCCESS) {
        paused_ = false;
        fail_streak_ = 0;
        last_feedback_ms_ = now;
      }
    }
  }
}
