#include "esp_wifi.h"

int32_t rssi = 0;

float readBattery() {
    int adc_read = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = (adc_read * 0.00511);
    return voltage;
  }
  
void promiscuous_rx_cb(void *buff, wifi_promiscuous_pkt_type_t type) {
  /*
  rssi < -90 dbm : muito ruim
  rssi ~ -65 dbm: sinal ok
  rssi > -55 dbm: sinal bom
  rssi > -30 dbm: sinal ótimo
  */
  if (type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  bool is_from_station = true;

  for (int i = 0; i < 6; i++) {
      if (hdr->addr2[i] != mac_address_station[i]) {
          is_from_station = false;
          break;
      }
  }
  if (is_from_station)  rssi = ppkt->rx_ctrl.rssi;
}