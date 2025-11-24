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
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
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
    
    // Configuración optimizada para rendimiento y QR
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565; // Escala de grises = Más rápido
    config.frame_size = FRAMESIZE_QVGA;        // Resolución ligera
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;   // Usar RAM externa
    config.jpeg_quality = 11; // se bajo la resolucion a 11; (12):predeterminado 
    config.fb_count = 1; // Solo 1 buffer para ahorrar memoria para el QR

    esp_camera_init(&config);
}

bool SystemModule::begin() {
    configCamera();
    reader->begin(); // Iniciar lógica del lector
    return true;
}

void SystemModule::startStreamServer() {
    startCameraServer(); 
}

String SystemModule::scanQR() {
    struct QRCodeData qr;
    
    if (reader->receiveQrCode(&qr, 100)) {
        if (qr.valid) {
            String resultado = String((const char *)qr.payload);
            return resultado;
        }
    }
    return "";
}