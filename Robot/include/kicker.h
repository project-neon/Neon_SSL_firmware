#pragma once

#include <cstdint>

void setup_kicker();
void charge_on();
void charge_off();
bool can_kick_now();
uint32_t calc_power(int pot);
bool do_kick(uint32_t pulse_us);
void kicker_control();
