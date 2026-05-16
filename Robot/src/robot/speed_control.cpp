#include <Arduino.h>
#include "speed_control.h"
#include "robot_context.h"
#include "kicker.h"

enum MotorIndex {
    MOTOR_RIGHT_DOWN = 1,
    MOTOR_RIGHT_TOP  = 2,
    MOTOR_LEFT_DOWN  = 3,
    MOTOR_LEFT_TOP   = 4,
};

void send_power(float m1, float m2, float m3, float m4)
{
    String result = "<0," + String(m1) + "," + String(m2) + ","
                  + "1," + String(m3) + "," + String(m4) + ">";
    Serial.println(result);
}

float calculate_motor(float v_x, float v_y, float angular, float L, float radius, int motor_index)
{
    switch (motor_index) {
    case MOTOR_RIGHT_DOWN:
        return (2.0f * L * angular - sqrt(3.0f) * v_x + v_y) / (2.0f * radius);
    case MOTOR_RIGHT_TOP:
        return (sqrt(2.0f) * L * angular - v_x - v_y) / (sqrt(2.0f) * radius);
    case MOTOR_LEFT_DOWN:
        return (sqrt(2.0f) * L * angular + v_x - v_y) / (sqrt(2.0f) * radius);
    case MOTOR_LEFT_TOP:
        return (2.0f * L * angular + sqrt(3.0f) * v_x + v_y) / (2.0f * radius);
    default:
        return 0.0f;
    }
}

float ramp_velocity(float target, float previous, float vel_step, float dt)
{
    float rate = abs(target - previous) / dt;

    if (rate > vel_step) {
        if (target > previous) {
            return previous + (vel_step * dt);
        } else {
            return previous - (vel_step * dt);
        }
    }
    return target;
}

void motors_control()
{
    float vel_rd = calculate_motor(robot->v_linear, robot->v_angular, robot->throttle,
                                    robot->wheelbase, robot->wheel_radius, MOTOR_RIGHT_DOWN);
    float vel_rt = calculate_motor(robot->v_linear, robot->v_angular, robot->throttle,
                                    robot->wheelbase, robot->wheel_radius, MOTOR_RIGHT_TOP);
    float vel_ld = calculate_motor(robot->v_linear, robot->v_angular, robot->throttle,
                                    robot->wheelbase, robot->wheel_radius, MOTOR_LEFT_DOWN);
    float vel_lt = calculate_motor(robot->v_linear, robot->v_angular, robot->throttle,
                                    robot->wheelbase, robot->wheel_radius, MOTOR_LEFT_TOP);

    vel_rd = ramp_velocity(vel_rd, robot->prev_rd, robot->vel_step, robot->dt);
    vel_rt = ramp_velocity(vel_rt, robot->prev_rt, robot->vel_step, robot->dt);
    vel_ld = ramp_velocity(vel_ld, robot->prev_ld, robot->vel_step, robot->dt);
    vel_lt = ramp_velocity(vel_lt, robot->prev_lt, robot->vel_step, robot->dt);

    send_power(vel_rd, vel_rt, vel_ld, vel_lt);

    robot->prev_rd = vel_rd;
    robot->prev_rt = vel_rt;
    robot->prev_ld = vel_ld;
    robot->prev_lt = vel_lt;
}

void fail_safe()
{
    robot->v_linear  = 0.0f;
    robot->v_angular = 0.0f;
    robot->throttle  = 0.0f;
    robot->last_error = 0;
    robot->error_sum  = 0.0f;
    robot->stop       = true;

    robot->kick_time      = 0;
    robot->waiting_to_kick = false;
    kicker_charge_off();
}
