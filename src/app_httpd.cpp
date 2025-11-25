#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "app_httpd.h"

// Variable global de estado
String ultimoMensaje = "SYSTEM BOOT... OK";

// Necesario para iniciar el servidor
httpd_handle_t stream_httpd = NULL;

void logToWeb(String mensaje) {
    ultimoMensaje = mensaje;
}

// ==========================================
// 1. HANDLER DE CAPTURA (OPTIMIZADO)
// ==========================================
static esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    
    fb = esp_camera_fb_get();
    if (!fb) {
        return ESP_FAIL;
    }

    // OPTIMIZACIÓN: Forzamos cierre de conexión para liberar sockets rápido
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close"); // <--- ESTO EVITA EL CONGELAMIENTO

    if (fb->format == PIXFORMAT_JPEG) {
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        size_t _jpg_buf_len = 0;
        uint8_t * _jpg_buf = NULL;
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        if (!jpeg_converted) {
            res = ESP_FAIL;
        } else {
            res = httpd_resp_send(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (_jpg_buf) free(_jpg_buf);
    }
    
    esp_camera_fb_return(fb);
    return res;
}

// ==========================================
// 2. HANDLER DE ESTADO
// ==========================================
static esp_err_t status_handler(httpd_req_t *req) {
    // Enviamos texto plano y cerramos conexión rápido
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close"); 
    httpd_resp_send(req, ultimoMensaje.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ==========================================
// 3. INTERFAZ SCI-FI (HTML + CSS + JS)
// ==========================================
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
<html lang="es">
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
        <title>VECTOR-X OS</title>
        <style>
            /* --- ESTILO CYBERPUNK --- */
            @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap');

            body { 
                background-color: #020202; 
                margin: 0; padding: 0; 
                overflow: hidden; 
                font-family: 'Share Tech Mono', monospace; /* Fuente Hacker */
                color: #00ffcc; 
                height: 100vh;
                display: flex; 
                flex-direction: column;
                align-items: center; 
                justify-content: center;
                background-image: radial-gradient(#111 15%, transparent 16%), radial-gradient(#111 15%, transparent 16%);
                background-size: 20px 20px;
            }

            /* --- MARCO DEL VISOR --- */
            #viewport {
                position: relative; 
                width: 95%; max-width: 720px; 
                aspect-ratio: 4/3;
                border: 1px solid #333; 
                background: #000;
                box-shadow: 0 0 20px rgba(0, 255, 204, 0.1);
            }

            /* IMAGEN VIDEO */
            img { 
                width: 100%; height: 100%; 
                object-fit: cover; 
                opacity: 0.8; 
                filter: contrast(1.2) sepia(0.2) hue-rotate(130deg) saturate(0.8); /* Efecto Vision Nocturna */
            }
            
            /* --- HUD OVERLAY (La capa de encima) --- */
            .hud { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; box-sizing: border-box; padding: 10px; }

            /* Decoraciones */
            .border-corner { position: absolute; width: 20px; height: 20px; border-color: #00ffcc; border-style: solid; transition: 0.3s; }
            .tl { top: 10px; left: 10px; border-width: 2px 0 0 2px; }
            .tr { top: 10px; right: 10px; border-width: 2px 2px 0 0; }
            .bl { bottom: 10px; left: 10px; border-width: 0 0 2px 2px; }
            .br { bottom: 10px; right: 10px; border-width: 0 2px 2px 0; }
            
            .crosshair {
                position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
                width: 40px; height: 40px; border: 1px dashed rgba(0, 255, 204, 0.3); border-radius: 50%;
            }
            .crosshair::after { content: '+'; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -55%); font-size: 20px; color: rgba(0, 255, 204, 0.6); }

            .scan-line {
                position: absolute; top: 0; left: 0; width: 100%; height: 2px;
                background: rgba(0, 255, 204, 0.5);
                box-shadow: 0 0 10px #00ffcc;
                animation: scanning 3s linear infinite;
                opacity: 0.5;
            }
            @keyframes scanning { 0% {top: 0%; opacity: 0;} 10% {opacity: 1;} 90% {opacity: 1;} 100% {top: 100%; opacity: 0;} }

            /* --- TERMINAL DE DATOS --- */
            #console-container {
                position: absolute; bottom: 20px; left: 50%; transform: translateX(-50%);
                width: 80%;
                background: rgba(0, 10, 0, 0.85);
                border: 1px solid #005544;
                padding: 10px;
                font-size: 14px;
                text-transform: uppercase;
                backdrop-filter: blur(2px);
            }

            .data-line { display: block; margin-bottom: 2px; }
            .label { color: #008866; font-size: 10px; }
            .value { font-weight: bold; letter-spacing: 1px; }
            
            #log-text { color: #00ffcc; text-shadow: 0 0 5px rgba(0, 255, 204, 0.5); }
            
            /* Estado de Alerta */
            .alert-state { 
                border-color: #ff3300 !important; 
                color: #ff3300 !important; 
                box-shadow: 0 0 15px rgba(255, 51, 0, 0.3) !important;
            }
            .alert-text { color: #ff3300 !important; text-shadow: 0 0 5px #ff3300 !important; }

        </style>
    </head>
    <body>
        <div id="viewport">
            <img id="video-feed" src="">
            
            <div class="hud">
                <div class="scan-line"></div>
                <div class="border-corner tl"></div>
                <div class="border-corner tr"></div>
                <div class="border-corner bl"></div>
                <div class="border-corner br"></div>
                <div class="crosshair"></div>

                <div style="position: absolute; top: 15px; left: 20px; font-size: 10px; background: #00ffcc; color: black; padding: 1px 5px;">CAM_01 ONLINE</div>

                <div id="console-container">
                    <span class="label">STATUS LOG:</span><br>
                    <span id="log-text" class="value">CONNECTING...</span>
                </div>
            </div>
        </div>

        <script>
            const img = document.getElementById('video-feed');
            const logBox = document.getElementById('log-text');
            const consoleDiv = document.getElementById('console-container');
            const corners = document.querySelectorAll('.border-corner');

            // 1. CONTROL DE VIDEO (THROTTLED - FRENO DE MANO)
            // Esta función evita el congelamiento
            function refreshImage() {
                // Timestamp evita caché
                img.src = "/capture?t=" + new Date().getTime();
            }
            
            // Cuando la imagen carga, ESPERAMOS 200ms antes de pedir la siguiente.
            // Esto libera al ESP32 para procesar QR y WiFi.
            img.onload = function() {
                setTimeout(refreshImage, 200); 
            };
            
            // Si falla, reintenta en 500ms
            img.onerror = function() {
                setTimeout(refreshImage, 500);
            };

            // Iniciar ciclo
            refreshImage();

            // 2. CONTROL DE DATOS (POLLING)
            setInterval(function() {
                fetch('/status')
                .then(response => response.text())
                .then(text => {
                    logBox.innerText = "> " + text;

                    // Efectos visuales de alerta
                    if(text.includes("ALERTA") || text.includes("DETENIDO")) {
                        consoleDiv.classList.add("alert-state");
                        logBox.classList.add("alert-text");
                        corners.forEach(c => c.style.borderColor = "#ff3300");
                    } else {
                        consoleDiv.classList.remove("alert-state");
                        logBox.classList.remove("alert-text");
                        corners.forEach(c => c.style.borderColor = "#00ffcc");
                    }
                })
                .catch(e => { 
                    logBox.innerText = "> NO SIGNAL"; 
                    logBox.style.color = "#555";
                });
            }, 1000); // Actualizamos texto cada 1 segundo (menos carga)
        </script>
    </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

// ==========================================
// 4. INICIAR SERVIDOR
// ==========================================
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 3; // Limitamos sockets para estabilidad

    httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
    httpd_uri_t status_uri = { .uri = "/status", .method = HTTP_GET, .handler = status_handler, .user_ctx = NULL };
    httpd_uri_t capture_uri = { .uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        httpd_register_uri_handler(stream_httpd, &status_uri);
        httpd_register_uri_handler(stream_httpd, &capture_uri);
    }
}