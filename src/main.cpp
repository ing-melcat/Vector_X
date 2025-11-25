#include <Arduino.h>
#include "config.h"
#include "RedWifi.h"
#include "SystemModule.h"
#include "Ultrasonico.h"
#include "motor.h"
#include "app_httpd.h" 

// --- INSTANCIAS ---
RedWifi miRed(Config::SSID, Config::PASSWORD);
SystemModule camara;
// Recordatorio: Canales PWM 12 y 13 para no chocar con cámara
Motor robot(Config::MTR_POS_IZQ, Config::MTR_NEG_IZQ, Config::MTR_POS_DER, Config::MTR_NEG_DER, Config::MTR_ENA, Config::MTR_ENB, Config::VELOCIDAD);
Ultrasonico ojos(Config::TRIG_PIN, Config::ECHO_PIN);

unsigned long tiempoLog = 0;

// --- LECTURA SENSOR ---
float leerDistancia() {
    float d = ojos.leer_distancia();
    // Filtro: Si es 0 o mayor a 4m, consideramos camino libre
    if (d == 0 || d > 400) return 999.0; 
    return d;
}

// --- SETUP ---
void setup() {
    // IMPORTANTE: No usamos Serial.begin porque los pines 1 y 3 son del sensor.
    
    // 1. Iniciar Punto de Acceso
    miRed.iniciarAP();
    
    // 2. Iniciar Sistemas de Visión
    if (camara.begin()) {
        camara.startStreamServer();
        logToWeb("SISTEMA INICIADO. BATERIA ESTABLE.");
    } else {
        // Si falla, intentamos avisar
        logToWeb("ERROR CRITICO EN MODULO DE CAMARA");
    }

    // 3. Asegurar robot detenido
    robot.detener();
}

// --- LOOP PRINCIPAL ---
void loop() {
    float distancia = leerDistancia();
    String mensajeQR = camara.scanQR();
    
    // --- GENERACIÓN DE REPORTE WEB ---
    // Actualizamos cada 300ms para no saturar la red
    if (millis() - tiempoLog > 300) {
        String reporte = "";
        
        // Formato profesional: DIST: [XX] cm | ESTADO
        reporte += "DIST: " + String((int)distancia) + " cm";
        reporte += " | ";
        
        if (mensajeQR != "") {
            reporte += "QR DETECTADO: " + mensajeQR;
        } else {
            // --- MODIFICACION VISUAL ---
            // Hacemos que parpadee para confirmar que el lector está vivo
            if ((millis() / 500) % 2 == 0) {
                reporte += "BUSCANDO [ * ]"; 
            } else {
                reporte += "BUSCANDO [   ]"; 
            }
        }
        
        logToWeb(reporte);
        tiempoLog = millis();
    }

    // --- LÓGICA DE CONTROL Y NAVEGACIÓN ---
    
    // CASO 1: OBSTÁCULO CERCANO (Prioridad Alta)
    if (distancia < Config::DISTANCIA_MINIMA) {
        robot.detener();
        logToWeb("[ALERTA] OBSTACULO CERCANO - DETENIDO");
    }
    
    // CASO 2: COMANDO QR RECIBIDO (Prioridad Media)
    else if (mensajeQR != "") {
        // Notificamos la acción
        String accion = "EJECUTANDO COMANDO: " + mensajeQR;
        logToWeb(accion);
        
        // Pausa de procesamiento
        robot.detener();
        delay(200);

        // Decodificación de comandos
        if (mensajeQR == "DERECHA") { 
            robot.derecha(); 
            delay(600); 
        }
        else if (mensajeQR == "IZQUIERDA") { 
            robot.izquierda(); 
            delay(600); 
        }
        else if (mensajeQR == "ATRAS") { 
            robot.retroceder(); 
            delay(800); 
        }
        else if (mensajeQR == "DETENER") { 
            robot.detener(); 
            delay(3000); 
        }
        else if (mensajeQR == "AVANZAR") { 
            robot.avanzar(); 
            delay(1000); 
        }

        // Finalizar comando
        robot.detener();
        logToWeb("ORDEN COMPLETADA. ESPERANDO...");
        
        // Tiempo muerto para evitar lecturas repetidas
        delay(1500); 
    }
    
    // CASO 3: NAVEGACIÓN POR DEFECTO (Prioridad Baja)
    else {
        robot.avanzar();
    }
    
    // Pequeño delay para estabilidad del Watchdog Timer del ESP32
    delay(10);
}