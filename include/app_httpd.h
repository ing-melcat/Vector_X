#ifndef APP_HTTPD_H
#define APP_HTTPD_H

#include <Arduino.h> // Para usar String

#ifdef __cplusplus
extern "C" {
#endif

void startCameraServer();

// Nueva función para enviar texto a la web
void logToWeb(String mensaje);

#ifdef __cplusplus
}
#endif

#endif