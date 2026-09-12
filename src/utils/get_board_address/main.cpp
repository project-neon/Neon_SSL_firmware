#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // Inicializa o Wi-Fi em modo station (pode ser também WIFI_AP ou WIFI_AP_STA)
  WiFi.mode(WIFI_STA);
}

void loop() {
  String macAddress = WiFi.macAddress();

  // Exibe o endereço MAC no monitor serial
  Serial.print("Endereço MAC: ");
  Serial.println(macAddress);

  delay(1500);
}