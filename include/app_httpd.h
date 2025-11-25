#ifndef APP_HTTPD_H
#define APP_HTTPD_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void startCameraServer();
void logToWeb(String mensaje); // <--- ESTA LINEA ES IMPORTANTE

#ifdef __cplusplus
}
#endif

#endif