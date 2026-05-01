#pragma once

#include <cstdint>
#include "config.h"

typedef struct
{
    int password;
    char message[MESSAGE_LENGTH_BYTES];
} struct_data;

typedef struct
{
    int password;
    int id;
    int rssi;
    float battery;
} struct_feedback;

typedef struct
{
    unsigned frame_ctrl : 16;
    unsigned duration_id : 16;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    unsigned sequence_ctrl : 16;
    uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} wifi_ieee80211_packet_t;
