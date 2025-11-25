#include "SystemModule.h"
#include "config.h"
#include "esp_http_server.h"
#include "app_httpd.h"

SystemModule::SystemModule() {
    // Inicialización del puntero del lector
    reader = new ESP32QRCodeReader(CAMERA_MODEL_AI_THINKER);
}

void SystemModule::configCamera() {
    camera_config_t config;

    // --- IMPORTANTE: CONFIGURACIÓN PWM ---
    // La cámara usa Canal 0 y Timer 0.
    // Esto es CORRECTO porque en 'motor.cpp' movimos los motores al Canal 2 y 3.
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    // Pines (vienen de config.h)
    config.pin_d0 = Config::Y2_GPIO_NUM;
    config.pin_d1 = Config::Y3_GPIO_NUM;
    config.pin_d2 = Config::Y4_GPIO_NUM;
    config.pin_d3 = Config::Y5_GPIO_NUM;
    config.pin_d4 = Config::Y6_GPIO_NUM;
    config.pin_d5 = Config::Y7_GPIO_NUM;
    config.pin_d6 = Config::Y8_GPIO_NUM;
    config.pin_d7 = Config::Y9_GPIO_NUM;
    config.pin_xclk = Config::XCLK_GPIO_NUM;
    config.pin_pclk = Config::PCLK_GPIO_NUM;
    config.pin_vsync = Config::VSYNC_GPIO_NUM;
    config.pin_href = Config::HREF_GPIO_NUM;
    config.pin_sccb_sda = Config::SIOD_GPIO_NUM;
    config.pin_sccb_scl = Config::SIOC_GPIO_NUM;
    config.pin_pwdn = Config::PWDN_GPIO_NUM;
    config.pin_reset = Config::RESET_GPIO_NUM;
    
    // --- OPTIMIZACIÓN CRÍTICA ---
    config.xclk_freq_hz = 20000000;
    
    // CAMBIO IMPORTANTE: Usamos GRAYSCALE en lugar de RGB565.
    // 1. Consume la mitad de memoria RAM.
    // 2. El QR se lee más rápido.
    // 3. El streaming de video fluye mejor por WiFi.
    config.pixel_format = PIXFORMAT_GRAYSCALE; 
    
    config.frame_size = FRAMESIZE_QVGA;        // 320x240 (Ideal para velocidad)
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;   // Usar RAM externa
    config.jpeg_quality = 12;                  // 12 es buen equilibrio, 10 o menos puede laggear
    config.fb_count = 1;                       // 1 Buffer está bien para ahorrar memoria

    // Inicializamos la cámara con NUESTRA configuración manual
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Error al iniciar cámara: 0x%x\n", err);
    }
}

bool SystemModule::begin() {
    // 1. Configuramos la cámara primero
    configCamera();

    // 2. Iniciamos el lector 
    // (Al ya estar iniciada la cámara arriba, el lector usará esa configuración)
    reader->begin(); 
    
    return true;
}

void SystemModule::startStreamServer() {
    startCameraServer(); 
}

String SystemModule::scanQR() {
    struct QRCodeData qr;
    
    // Intentamos leer el QR
    if (reader->receiveQrCode(&qr, 100)) {
        if (qr.valid) {
            String resultado = String((const char *)qr.payload);
            // Pequeño debug para saber que leyó algo
            // Serial.println("QR Detectado: " + resultado); 
            return resultado;
        }
    }
    return "";
}