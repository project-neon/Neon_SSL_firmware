#pragma once

#include <esp_now.h>

#include <cstdint>
#include <cstring>

#include "config.h"
#include "types.h"

class RobotContext {
 public:
  RobotContext(int id, bool fb, bool rssi_en, const uint8_t mac_fb[6],
               const uint8_t mac_st[6], float wbase, float wradius, float step);

  // ── Setup ──────────────────────────────────────────
  void setup_kicker_pins();
  void begin_charge();
  void setup_peer(uint8_t channel);
  bool rssi_enabled() const { return compute_rssi_; }
  const uint8_t *feedback_mac() const { return mac_feedback_; }
  esp_now_peer_info_t *peer_info() { return &peer_; }

  // ── ESP-NOW callbacks (thin free-functions delegate to these) ──
  void handle_received_packet(const uint8_t *data, int len);
  void handle_send_status(esp_now_send_status_t status);

  // ── Sensor callback ─────────────────────────────────
  void set_rssi(int32_t r) { rssi_ = r; }
  const uint8_t *station_mac() const { return mac_station_; }

  // ── Loop: command processing ──────────────────────
  bool has_pending_command() const { return new_data_; }
  void process_command();

  // ── Loop: safety ───────────────────────────────────
  bool is_stopped() const { return stop_; }
  int last_command_ms() const { return last_command_ms_; }
  void fail_safe_check();

  // ── Loop: subsystems (called each tick) ────────────
  void kicker_update();
  void update_timing();
  void motors_update();
  void feedback_update();

 private:
  // ── Configuration ──────────────────────────────────
  int robot_id_;
  bool use_feedback_;
  bool compute_rssi_;
  uint8_t mac_feedback_[6];
  uint8_t mac_station_[6];
  float wheelbase_;
  float wheel_radius_;
  float vel_step_;

  // ── Command buffers ────────────────────────────────
  char command_buffer_[MESSAGE_LENGTH_BYTES] = {};
  char scratch_buffer_[MESSAGE_LENGTH_BYTES] = {};
  int parsed_id_ = 0;
  bool new_data_ = false;

  // ── Motion commands ────────────────────────────────
  bool stop_ = false;
  float v_linear_ = 0.0f;
  float v_angular_ = 0.0f;
  float throttle_ = 0.0f;

  // ── Kicker ─────────────────────────────────────────
  int kick_time_ = 0;
  bool waiting_to_kick_ = false;
  bool charge_enabled_ = false;
  unsigned long charge_on_since_ms_ = 0;
  unsigned long can_kick_since_ = 0;
  unsigned long kick_done_ms_ = 0;
  bool waiting_before_recharge_ = false;

  // ── Motor state ────────────────────────────────────
  float prev_rd_ = 0.0f, prev_rt_ = 0.0f, prev_ld_ = 0.0f, prev_lt_ = 0.0f;
  int last_error_ = 0;
  float error_sum_ = 0.0f;

  // ── Timing ─────────────────────────────────────────
  int last_command_ms_ = 0;
  float dt_ = 0.0f;
  int last_time_ = 0;

  // ── Feedback ───────────────────────────────────────
  uint32_t last_feedback_ms_ = 0;
  uint32_t last_probe_ms_ = 0;
  uint32_t pause_until_ms_ = 0;
  bool paused_ = false;
  int fail_streak_ = 0;
  int32_t rssi_ = 0;
  esp_now_send_status_t last_send_status_ = ESP_NOW_SEND_FAIL;

  // ── ESP-NOW structures ─────────────────────────────
  robot_command data_received_ = {};
  feedback_message data_feedback_ = {};
  esp_now_peer_info_t peer_ = {};

  // ═══════════════════════════════════════════════════
  // Private helpers
  // ═══════════════════════════════════════════════════

  // Kicker
  void end_charge();
  bool can_fire_now(unsigned long now) const;
  void do_kick_pulse(uint32_t us);
  uint32_t kick_pulse_us(int power) const;

  // Motor
  float motor_velocity(int motor_idx, float vx, float vy, float va) const;
  float ramp(float target, float prev, float dt) const;
  void send_powers(float rd, float rt, float ld, float lt);

  // Command
  void parse_command_internal();

  // Feedback
  bool send_feedback_packet();
};

extern RobotContext *robot;
