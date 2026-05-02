#include <Arduino.h>
#include "kicker.h"
#include "robot_context.h"

void setup_kicker()
{
    pinMode(KICKER_PIN, OUTPUT);
    pinMode(CHARGE_KICKER_PIN, OUTPUT);
    digitalWrite(KICKER_PIN, LOW);
    digitalWrite(CHARGE_KICKER_PIN, LOW);
    charge_on();
}

void charge_on()
{
    if (!robot->charge_enabled)
    {
        digitalWrite(CHARGE_KICKER_PIN, HIGH);
        robot->charge_enabled = true;
        robot->charge_on_since_ms = millis();
    }
}

void charge_off()
{
    if (robot->charge_enabled)
    {
        digitalWrite(CHARGE_KICKER_PIN, LOW);
        robot->charge_enabled = false;
    }
}

bool can_kick_now()
{
    unsigned long now = millis();
    if (!robot->charge_enabled)
        return false;
    if (now - robot->charge_on_since_ms < MIN_CHARGE_TIME_MS)
        return false;
    return true;
}

uint32_t calc_power(int pot)
{
    return (uint32_t)max(0, min(20000, (int)map(pot, 0, 9, 0, 20000)));
}

bool do_kick(uint32_t pulse_us)
{
    digitalWrite(KICKER_PIN, HIGH);
    delayMicroseconds(pulse_us);
    digitalWrite(KICKER_PIN, LOW);
    return true;
}

void kicker_control()
{
    unsigned long now = millis();

    if (robot->waiting_before_recharge)
    {
        if (now - robot->kick_done_ms >= TIME_BEFORE_CHARGE_MS)
        {
            if (!robot->charge_enabled)
            {
                charge_on();
            }
            robot->waiting_before_recharge = false;
        }
        return;
    }

    if (robot->kick_time <= 0)
    {
        if (!robot->charge_enabled)
        {
            charge_on();
        }
        robot->waiting_to_kick = false;
        return;
    }

    if (!robot->waiting_to_kick)
    {
        if (can_kick_now())
        {
            charge_off();
            robot->can_kick_since = now;
            robot->waiting_to_kick = true;
        }
        return;
    }

    if (now - robot->can_kick_since >= TIME_AFTER_CHARGE_MS)
    {
        if (do_kick(calc_power(robot->kick_time)))
        {
            robot->kick_time = 0;
            robot->waiting_to_kick = false;
            robot->waiting_before_recharge = true;
            robot->kick_done_ms = now;
        }
    }
}
