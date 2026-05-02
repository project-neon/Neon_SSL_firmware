#pragma once

void send_power(float m1, float m2, float m3, float m4);
float calculate_motor(float v_x, float v_y, float angular, float L, float radius, int motor_index);
float ramp_velocity(float target, float previous, float vel_step, float dt);
void motors_control();
void fail_safe();
