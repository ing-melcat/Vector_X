#ifndef SYSTEMMODULE_H
#define SYSTEMMODULE_H

// Librerías necesarias
#include <Arduino.h>
#include "esp_camera.h"
#include "ESP32QRCodeReader.h" 

class SystemModule {
public:
    SystemModule();
    
    bool begin();
    void startStreamServer();
    String scanQR(); // Devuelve el texto del QR o "" si no hay nada

private:
    void configCamera();
    
    // Puntero al lector de QR
    ESP32QRCodeReader* reader = nullptr;
};

#endif