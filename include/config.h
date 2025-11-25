#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

namespace Config {
    // --- WIFI ---
    static const char* SSID = "Vector-X";
    static const char* PASSWORD = "Unipoli_2026";

    // --- PINES DE DIRECCIÓN MOTORES ---
    static const int MTR_POS_IZQ = 12; 
    static const int MTR_NEG_IZQ = 13;
    static const int MTR_POS_DER = 2;
    static const int MTR_NEG_DER = 4;

    // --- PINES DE VELOCIDAD/POTENCIA (PWM) ---
    static const int MTR_ENA = 15; 
    static const int MTR_ENB = 14;
    
    // Velocidad global (0-255) - 180 es una buena velocidad media/baja
    static const int VELOCIDAD = 60;

    // --- PINES ULTRASONICO ---
    static const int TRIG_PIN = 0;  // TX0 (Cuidado al subir código)
    static const int ECHO_PIN = 3;
    static const float DISTANCIA_MINIMA = 100.0; // Ajustado a 25cm para frenar con tiempo

    // --- PINES CAMARA (ESP32-CAM AI THINKER) ---
    static const int PWDN_GPIO_NUM  = 32;
    static const int RESET_GPIO_NUM = -1;
    static const int XCLK_GPIO_NUM  = 0;
    static const int SIOD_GPIO_NUM  = 26;
    static const int SIOC_GPIO_NUM  = 27;
    static const int Y9_GPIO_NUM    = 35;
    static const int Y8_GPIO_NUM    = 34;
    static const int Y7_GPIO_NUM    = 39;
    static const int Y6_GPIO_NUM    = 36;
    static const int Y5_GPIO_NUM    = 21;
    static const int Y4_GPIO_NUM    = 19;
    static const int Y3_GPIO_NUM    = 18;
    static const int Y2_GPIO_NUM    = 5;
    static const int VSYNC_GPIO_NUM = 25;
    static const int HREF_GPIO_NUM  = 23;
    static const int PCLK_GPIO_NUM  = 22;
}
#endif