#include <Arduino.h>
#include "sensor.h"
#include "robot_context.h"
#include "types.h"

float read_battery()
{
    int adc_read = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = adc_read * 0.00511f;
    return voltage;
}

void promiscuous_rx_cb(void* buff, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_MGMT) {
        return;
    }

    const wifi_promiscuous_pkt_t* ppkt = (wifi_promiscuous_pkt_t*)buff;
    const wifi_ieee80211_packet_t* ipkt = (wifi_ieee80211_packet_t*)ppkt->payload;
    const wifi_ieee80211_mac_hdr_t* hdr = &ipkt->hdr;

    const uint8_t* sta_mac = robot->station_mac();
    bool from_station = true;
    for (int i = 0; i < 6; i++) {
        if (hdr->addr2[i] != sta_mac[i]) {
            from_station = false;
            break;
        }
    }

    if (from_station) {
        robot->set_rssi(ppkt->rx_ctrl.rssi);
    }
}
