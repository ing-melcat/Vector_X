#include <Arduino.h>
#include "config.h"
#include "RedWifi.h"
#include "SystemModule.h"
#include "Ultrasonico.h"
#include "motor.h"
#include "app_httpd.h" // Importante incluir esto para usar logToWeb

// ==========================================
// INSTANCIAS
// ==========================================
RedWifi miRed(Config::SSID, Config::PASSWORD);
SystemModule camara;
Motor robot(
    Config::MTR_POS_IZQ, Config::MTR_NEG_IZQ, 
    Config::MTR_POS_DER, Config::MTR_NEG_DER,
    Config::MTR_ENA, Config::MTR_ENB, Config::VELOCIDAD
);
Ultrasonico ojos(Config::TRIG_PIN, Config::ECHO_PIN);

unsigned long ultimoLog = 0;

// ==========================================
// AUXILIARES
// ==========================================
float leerDistanciaSegura() {
    float d = ojos.leer_distancia();
    if (d == 0) return 999.0; 
    return d;
}

// ==========================================
// SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000); 

    // Mensaje inicial a la web (aunque nadie lo verá hasta conectarse)
    logToWeb("Iniciando Sistemas...");

    miRed.iniciarAP();
    
    if (camara.begin()) {
        camara.startStreamServer();
        logToWeb("Camara Lista. Esperando Comandos.");
    }

    robot.detener();
}

// ==========================================
// LOOP
// ==========================================
void loop() {
    float distancia = leerDistanciaSegura();
    String mensajeQR = camara.scanQR();

    // Lógica para enviar mensajes a la Web y al Serial
    // Lo hacemos condicional para no saturar
    if (millis() - ultimoLog > 500) {
        String estado = "D: " + String((int)distancia) + "cm";
        
        if (mensajeQR != "") {
            estado += " | QR: " + mensajeQR;
        } else {
            estado += " | Escaneando...";
        }

        // --- AQUÍ OCURRE LA MAGIA ---
        // Esto envía el texto a la cajita negra de la página web
        logToWeb(estado); 
        // ----------------------------
        
        Serial.println(estado); // Tambien al cable por si acaso
        ultimoLog = millis();
    }

    // --- CONTROL ---
    if (distancia < Config::DISTANCIA_MINIMA) {
        robot.detener();
        logToWeb("⛔ OBSTACULO DETECTADO ⛔"); // Alerta visual en web
    }
    else if (mensajeQR != "") {
        String accion = "EJECUTANDO: " + mensajeQR;
        logToWeb(accion); // Mostrar acción en web
        Serial.println(accion);
        
        robot.detener();
        delay(100);

        if (mensajeQR == "DERECHA") {
            robot.derecha(); delay(600);
        }
        else if (mensajeQR == "IZQUIERDA") {
            robot.izquierda(); delay(600);
        }
        else if (mensajeQR == "ATRAS") {
            robot.retroceder(); delay(1000);
        }
        else if (mensajeQR == "DETENER") {
            robot.detener(); delay(3000);
        }
        else if (mensajeQR == "AVANZAR") {
            robot.avanzar(); delay(1000);
        }
        
        robot.detener();
        logToWeb("Orden Finalizada. Esperando...");
        delay(1500);
    }
    else {
        robot.avanzar();
    }

    delay(10); 
}