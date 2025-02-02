#include "esp_camera.h"
#include <WiFi.h>
#include "esp_eap_client.h" // WPA2-Enterprise support

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_AI_THINKER  // Change this if needed
#include "camera_pins.h"

// ===========================
// Eduroam Credentials
// ===========================
#define EAP_ANONYMOUS_IDENTITY ""
#define EAP_IDENTITY ""
#define EAP_PASSWORD ""
#define SSID "eduroam"

void startCameraServer();

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting ESP32-CAM...");

    // ===========================
    // Camera Initialization
    // ===========================
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_QVGA;  // Keep resolution small for stability
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed! Error 0x%x\n", err);
        return;
    }

    Serial.println("Camera initialized!");

    // ===========================
    // Connect to Eduroam
    // ===========================
    Serial.print("Connecting to: ");
    Serial.println(SSID);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // ===========================
    // Start Camera Server
    // ===========================
    startCameraServer();
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
}

void loop() {
    delay(10000);
}
