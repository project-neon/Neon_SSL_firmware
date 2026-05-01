#include <Arduino.h>
#include "speed_control.h"
#include "robot_context.h"
#include "kicker.h"

void send_power(float m1, float m2, float m3, float m4)
{
    String result = "<0," + String(m1) + "," + String(m2) + "," + "1," + String(m3) + "," + String(m4) + ">";
    Serial.println(result);
}

float calculate_motor(float v_x, float v_y, float angular, float L, float radius, int motor)
{
    float vel = 0.0f;

    if (motor == 1)
    {
        vel = (2.0f * L * angular - sqrt(3.0f) * v_x + v_y) / (2.0f * radius);
    }
    if (motor == 2)
    {
        vel = (sqrt(2.0f) * L * angular - v_x - v_y) / (sqrt(2.0f) * radius);
    }
    if (motor == 3)
    {
        vel = (sqrt(2.0f) * L * angular + v_x - v_y) / (sqrt(2.0f) * radius);
    }
    if (motor == 4)
    {
        vel = (2.0f * L * angular + sqrt(3.0f) * v_x + v_y) / (2.0f * radius);
    }

    return vel;
}

float acceleration(float m, float previous_m, float vel_step, float dt)
{
    float rate = abs(m - previous_m) / dt;

    if (rate > vel_step)
    {
        if (m > previous_m)
        {
            m = previous_m + (vel_step * dt);
        }
        else
        {
            m = previous_m - (vel_step * dt);
        }
    }
    return m;
}

void motors_control()
{
    float vel_rd = calculate_motor(robot->v_l, robot->v_a, robot->th, robot->L, robot->r, 1);
    float vel_rt = calculate_motor(robot->v_l, robot->v_a, robot->th, robot->L, robot->r, 2);
    float vel_ld = calculate_motor(robot->v_l, robot->v_a, robot->th, robot->L, robot->r, 3);
    float vel_lt = calculate_motor(robot->v_l, robot->v_a, robot->th, robot->L, robot->r, 4);

    vel_rd = acceleration(vel_rd, robot->prev_rd, robot->vel_step, robot->dt);
    vel_rt = acceleration(vel_rt, robot->prev_rt, robot->vel_step, robot->dt);
    vel_ld = acceleration(vel_ld, robot->prev_ld, robot->vel_step, robot->dt);
    vel_lt = acceleration(vel_lt, robot->prev_lt, robot->vel_step, robot->dt);

    send_power(vel_rd, vel_rt, vel_ld, vel_lt);

    robot->prev_rd = vel_rd;
    robot->prev_rt = vel_rt;
    robot->prev_ld = vel_ld;
    robot->prev_lt = vel_lt;
}

void fail_safe()
{
    robot->v_l = 0.0f;
    robot->v_a = 0.0f;
    robot->th = 0.0f;
    robot->last_error = 0;
    robot->error_sum = 0.0f;
    robot->stop = true;

    robot->kick_time = 0;
    robot->waiting_to_kick = false;
    charge_off();
}
