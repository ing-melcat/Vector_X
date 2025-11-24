#ifndef REDWIFI_H
#define REDWIFI_H

#include <Arduino.h>
#include <WiFi.h>

class RedWifi {
    private:
        const char* _ssid;
        const char* _password;
        
    public:
        RedWifi(const char* ssid, const char* password);
        
        void iniciarAP();      // Iniciar Access Point
        IPAddress obtenerIP(); // Obtener dirección IP
};

#endif