#include <WiFi.h> 
//3 FITAS: A4:CF:12:72:B7:20
//2 fitas: 08:B6:1F:28:E3:94
//1 fita: 24:0A:C4:82:93:04

//cabo preto: EC:62:60:9A:A1:00 (sender)

//cabo branco: EC:62:60:94:CF:10 (receiver)

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);

  // Inicializa o Wi-Fi em modo station (pode ser também WIFI_AP ou WIFI_AP_STA)
  WiFi.mode(WIFI_STA);

  // Obtém o endereço MAC
  String macAddress = WiFi.macAddress();

  // Exibe o endereço MAC no monitor serial
  Serial.print("Endereço MAC: ");
  Serial.println(macAddress);
}

void loop() {
  // Não há necessidade de fazer nada no loop
  String macAddress = WiFi.macAddress();

  // Exibe o endereço MAC no monitor serial
  Serial.print("Endereço MAC: ");
  Serial.println(macAddress);

}