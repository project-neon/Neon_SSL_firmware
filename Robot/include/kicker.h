#pragma once

#include <cstdint>

void setup_kicker_pins();
void kicker_charge_on();
void kicker_charge_off();
bool can_kick_now();
uint32_t calc_power(int pot);
bool do_kick(uint32_t pulse_us);
void kicker_control();
