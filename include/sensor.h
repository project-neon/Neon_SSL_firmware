#pragma once

#include <esp_wifi.h>

float read_battery();
void promiscuous_rx_cb(void *buff, wifi_promiscuous_pkt_type_t type);
