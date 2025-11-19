#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ********** WiFi credentials **********
const char* ssid = "faruq";
const char* password = "aimanhensem123";

// ********** Streaming constants **********
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// ********** Server handles **********
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
int stream_port_number = 80; // Use standard HTTP port

// ********** Servo constants **********
const int servo_pin = 2;       // Servo connected to GPIO2
const int button_pin = 14;     // Push button connected to GPIO14
const int servo_min_position = 1000;
const int servo_max_position = 5000;
const int servo_speed = 50;
int servo_position = servo_min_position;

// ********** Stream handler **********
static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[64];
    static int64_t last_frame = 0;

    if (!last_frame) last_frame = esp_timer_get_time();

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted) {
                    Serial.println("JPEG compression failed");
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }

        if (res == ESP_OK) {
            size_t hlen = snprintf(part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        }
        if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        if (res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));

        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        if (res != ESP_OK) break;

        // Servo movement with button
        bool button_pressed = digitalRead(button_pin) == LOW; // LOW = pressed
        if (button_pressed) {
            servo_position += servo_speed;
            if (servo_position > servo_max_position) servo_position = servo_max_position;
        } else {
            servo_position -= servo_speed;
            if (servo_position < servo_min_position) servo_position = servo_min_position;
        }
        ledcWrite(0, servo_position);
    }

    last_frame = 0;
    return res;
}

// ********** Web page handler **********
static esp_err_t web_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");

    char webPage[250] = "";
    sprintf(webPage, "<!doctype html><html><head><title>ESP32-CAM</title></head>"
                     "<body><h2>ESP32-CAM Streaming</h2>"
                     "<p>Direct MJPEG stream: <a href='http://%d.%d.%d.%d/stream'>http://%d.%d.%d.%d/stream</a></p>"
                     "<img src='http://%d.%d.%d.%d/stream'>"
                     "</body></html>",
            WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3],
            WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3],
            WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);

    return httpd_resp_send(req, webPage, strlen(webPage));
}

// ********** Start camera server **********
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Web page
    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = web_handler,
        .user_ctx = NULL
    };

    // MJPEG stream
    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &stream_uri);
    }
}

// ********** Setup **********
void setup() {
    Serial.begin(115200);
    pinMode(button_pin, INPUT_PULLUP);

    // Servo PWM setup
    ledcSetup(0, 50, 16);
    ledcAttachPin(servo_pin, 0);
    ledcWrite(0, servo_position);

    // Connect WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize AI-Thinker ESP32-CAM
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }

    startCameraServer();

    Serial.println("Camera ready! Access via browser:");
    Serial.printf("http://%d.%d.%d.%d/\n", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    Serial.printf("Direct MJPEG stream: http://%d.%d.%d.%d/stream\n", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
}

void loop() {
    // Streaming runs in stream_handler
}