#include "RedWifi.h"

RedWifi::RedWifi(const char* ssid, const char* password) {
    this->_ssid = ssid;
    this->_password = password;
}

void RedWifi::iniciarAP() {
    WiFi.mode(WIFI_AP);
    bool resultado = WiFi.softAP(_ssid, _password);
    
    if (resultado) {
        Serial.println("AP Creado: " + String(_ssid));
    } else {
        Serial.println("Error creando AP");
    }
}

IPAddress RedWifi::obtenerIP() {
    return WiFi.softAPIP();
}