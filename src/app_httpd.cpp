#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "app_httpd.h"

// Variable global para guardar el último mensaje del robot
String mensajeGlobal = "Sistema Iniciado. Esperando datos...";

// Función que usaremos desde el main para actualizar el mensaje
void logToWeb(String mensaje) {
    mensajeGlobal = mensaje;
}

// ==========================================
// 1. HANDLER DEL VIDEO (STREAM)
// ==========================================
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            res = ESP_FAIL;
        } else {
            if (fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted) res = ESP_FAIL;
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK) break;
    }
    return res;
}

// ==========================================
// 2. HANDLER DE ESTADO (TEXTO)
// ==========================================
// La página web llamará a esto cada 500ms para pedir texto nuevo
static esp_err_t status_handler(httpd_req_t *req) {
    httpd_resp_send(req, mensajeGlobal.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ==========================================
// 3. HANDLER DE LA PÁGINA PRINCIPAL (HTML)
// ==========================================
// Aquí diseñamos la interfaz gráfica
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>Robot Vector-X Control</title>
        <style>
            body { background-color: #1a1a1a; color: white; font-family: sans-serif; text-align: center; margin: 0; }
            h1 { margin: 10px; color: #00ffcc; text-shadow: 0 0 5px #00ffcc; }
            #cam-container { position: relative; display: inline-block; width: 100%; max-width: 640px; }
            img { width: 100%; border: 2px solid #333; border-radius: 8px; }
            
            /* CONSOLA DE DATOS */
            #console-box {
                background-color: #000;
                border: 1px solid #00ff00;
                color: #00ff00;
                font-family: 'Courier New', monospace;
                padding: 10px;
                margin: 10px auto;
                width: 90%;
                max-width: 600px;
                height: 60px;
                border-radius: 5px;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 1.1em;
                box-shadow: 0 0 10px rgba(0, 255, 0, 0.2);
            }
        </style>
    </head>
    <body>
        <h1>VECTOR-X VISION</h1>
        
        <div id="cam-container">
            <img src="/stream" id="stream">
        </div>

        <div id="console-box">Cargando datos...</div>

        <script>
            // Script para actualizar el texto sin recargar la página
            setInterval(function() {
                fetch('/status')
                .then(response => response.text())
                .then(text => {
                    document.getElementById('console-box').innerHTML = "> " + text;
                });
            }, 500); // Se actualiza cada medio segundo
        </script>
    </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

// ==========================================
// 4. INICIO DEL SERVIDOR
// ==========================================
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t status_uri = {
        .uri       = "/status",
        .method    = HTTP_GET,
        .handler   = status_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        httpd_register_uri_handler(stream_httpd, &status_uri);
        Serial.println("Servidor Web Iniciado con Consola");
    }
}