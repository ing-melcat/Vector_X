#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
// EVITAR PROBLEMAS DE INESTABILIDADA
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "config.h"
#include "RedWifi.h"
#include "SystemModule.h"
#include "Ultrasonico.h" // Usamos tu librería original
#include "motor.h"

// ==========================================
// 1. INSTANCIAS GLOBALES
// ==========================================
RedWifi miRed(Config::SSID, Config::PASSWORD);
SystemModule camara;
Ultrasonico ojos(Config::TRIG_PIN, Config::ECHO_PIN);

Motor robot(
    Config::MTR_POS_IZQ, Config::MTR_NEG_IZQ, 
    Config::MTR_POS_DER, Config::MTR_NEG_DER,
    Config::MTR_ENA, Config::MTR_ENB, Config::VELOCIDAD     
);

// --- CONSOLA WEB (Puerto 81) ---
WiFiServer logServer(81);
String webLogBuffer = ""; 

// Variables de Control y Memoria
String ultimoEstadoImpreso = ""; 
float ultimaDistanciaValida = 100.0; 
bool camaraOnline = false;

// ==========================================
// 2. FUNCIÓN LOG PARA EL ENVIO DE MENSAJES A LA CONSOLA
// ==========================================
void logSistema(String mensaje) {
    // 1. USB
    Serial.println(mensaje);
    
    // 2. Web (Buffer circular)
    String tiempo = "[" + String(millis()/1000) + "s] ";
    webLogBuffer += tiempo + mensaje + "\n";
    
    // Limpieza de memoria para no saturar RAM
    if (webLogBuffer.length() > 1200) {
        webLogBuffer = webLogBuffer.substring(webLogBuffer.length() - 1000);
    }
}

// ==========================================
// 3. LECTURA DE SENSORES
// ==========================================
// Esta versión es más lenta pero MUCHO más confiable que la Turbo
float leerDistanciaSegura() {
    float total = 0;
    int validas = 0;
    
    // Tomamos 3 lecturas y promediamos
    for (int i = 0; i < 3; i++) {
        float d = ojos.leer_distancia();
        
        // Filtramos errores (0.0 o lecturas infinitas)
        if (d > 1.0 && d < 400.0) { 
            total += d;
            validas++;
        }
        delay(10); // Pequeña pausa necesaria para estabilidad
    }
    
    // Si el sensor falla, devolvemos la última conocida para no frenar en falso
    if (validas == 0) return ultimaDistanciaValida; 
    
    float promedio = total / validas;
    ultimaDistanciaValida = promedio;
    return promedio;
}

// ==========================================
// 4. SERVIDOR WEB (HTML)
// ==========================================
void atenderClienteWeb() {
    WiFiClient client = logServer.available();
    if (client) {
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println("Refresh: 3"); // Refresco automático suave
                    client.println();
                    client.println("<!DOCTYPE HTML><html><head>");
                    client.println("<style>body{background:#111;color:#0f0;font-family:monospace;padding:20px;} h2{border-bottom:1px solid #0f0;}</style>");
                    client.println("</head><body><h2>>> UNIPOLI SYSTEM MONITOR</h2><pre>");
                    client.println(webLogBuffer);
                    client.println("</pre></body></html>");
                    break;
                }
                if (c == '\n') { currentLineIsBlank = true; }
                else if (c != '\r') { currentLineIsBlank = false; }
            }
        }
        delay(1);
        client.stop();
    }
}

// ==========================================
// 5. SETUP (ESTABILIZADO)
// ==========================================
void setup() {
    // DESACTIVAR DETECTOR DE BAJONES DE LUZ (Evita reinicios)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

    Serial.begin(115200);
    // Espera para cargar capacitores
    delay(2000); 

    logSistema("\n\n========================================");
    logSistema("   SYSTEM BOOT: PROTOTPO SILLA DE RUEDAS");
    logSistema("========================================");

    // 1. WIFI
    miRed.iniciarAP();
    logSistema("[NET] AP Creado. Conectando...");
    
    // Construimos el mensaje IP con seguridad
    String ipMsg = "[NET] WEB CONSOLE: http://" + miRed.obtenerIP().toString() + ":81";
    logSistema(ipMsg);

    // Iniciamos servidor web
    logServer.begin();

    // Pausa de seguridad antes de encender periféricos pesados
    delay(500); 

    // 2. CÁMARA
    if (camara.begin()) {
        camara.startStreamServer();
        camaraOnline = true;
        logSistema("========================================  ");
        logSistema("[CAM] STATUS: ONLINE (Video Ready)");
    } else {
        camaraOnline = false;
        logSistema(" ======================================= ");
        logSistema("[CAM] STATUS: FAILED (Check hardware)");
    }

    // 3. MOTORES
    robot.detener();
    logSistema("=========================================== ");
    logSistema("[SYS] SYSTEM READY >> STARTING MOTION");
    delay(1000);
}

// ==========================================
// 6. LOOP PRINCIPAL
// ==========================================
void loop() {
    // 1. Revisar Web
    atenderClienteWeb();

    // 2. Leer Sensores (Con filtro estable)
    float distancia = leerDistanciaSegura();
    
    // 3. Leer QR (Solo si cámara funciona)
    String qr = "";
    if (camaraOnline) {
        qr = camara.scanQR();
    }

    // --- LÓGICA DE CONTROL ---

    // A. OBSTÁCULO (Seguridad)
    // Usamos > 1.0 para filtrar ruido eléctrico
    if (distancia < Config::DISTANCIA_MINIMA && distancia > 1.0) {
        if (ultimoEstadoImpreso != "STOP") {
            robot.detener();
            logSistema(" [ALERT] OBSTACLE DETECTED: " + String(distancia) + " cm");
            ultimoEstadoImpreso = "STOP";
        }
    }

    // B. COMANDO QR
    else if (qr != "") {
        robot.detener();
        logSistema(" [QR] COMMAND RECEIVED: " + qr);
        
        // Tiempos normales para asegurar el movimiento
        if (qr == "DERECHA")        { robot.derecha(); delay(600); }
        else if (qr == "IZQUIERDA") { robot.izquierda(); delay(600); }
        else if (qr == "ATRAS")     { robot.retroceder(); delay(600); }
        else if (qr == "DETENER")   { robot.detener(); delay(500); }
        
        robot.detener();
        logSistema("[QR] ACTION COMPLETED >> RESUMING");
        ultimoEstadoImpreso = "WAITING"; 
        delay(500); // Pausa post-comando
    }

    // C. PATRULLAJE
    else {
        if (ultimoEstadoImpreso != "GO") {
            robot.avanzar();
            logSistema(" [AUTO] PATH CLEAR >> MOVING, SEARCHING QR CODE");
            ultimoEstadoImpreso = "GO";
        }
    }

    // DELAY NECESARIO PARA ESTABILIDAD
    // Esto evita que el procesador se caliente o se reinicie
    delay(50); 
}