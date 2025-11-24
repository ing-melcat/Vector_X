#ifndef SYSTEMMODULE_H
#define SYSTEMMODULE_H

#include <Arduino.h>
#include "esp_camera.h"
#include "ESP32QRCodeReader.h" 

class SystemModule {
public:
    SystemModule();
    bool begin();
    void startStreamServer();
    String scanQR(); 

private:
    void configCamera();
    
    // Puntero al lector QR para evitar conflictos de inicialización
    ESP32QRCodeReader* reader = nullptr;
};

#endif