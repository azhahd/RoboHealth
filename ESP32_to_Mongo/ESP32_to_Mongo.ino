#include "esp_camera.h"
#include <WiFi.h>
#include "esp_eap_client.h"  // WPA2-Enterprise support
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>

// ===================
// Select Camera Model
// ===================
#define CAMERA_MODEL_AI_THINKER  
#include "camera_pins.h"

// ===========================
// Eduroam or Custom Wi-Fi Credentials
// ===========================
#define EAP_ANONYMOUS_IDENTITY "j239cai@uwaterloo.ca"
#define EAP_IDENTITY "j239cai@uwaterloo.ca"
#define EAP_PASSWORD "342198264jJ."
#define SSID "eduroam"

// ===========================
// Static IP Configuration
// ===========================
IPAddress local_IP(100, 67, 192, 50);   // Pick an available IP in the network
IPAddress gateway(100, 67, 192, 1);     // Default Gateway
IPAddress subnet(255, 255, 224, 0);     // Subnet Mask
IPAddress primaryDNS(8, 8, 8, 8);       // Google DNS
IPAddress secondaryDNS(8, 8, 4, 4);     // Backup DNS

// ===========================
// FastAPI Server IP
// ===========================
const char* serverUrl = "http://100.67.223.239:8000/upload/"; // Replace with your actual FastAPI server IP

// Web server for ESP32-CAM UI
AsyncWebServer server(80);

// ===========================
// Capture and Send Image Function
// ===========================
void sendCapturedImage() {
    WiFiClient client;
    HTTPClient http;

    Serial.println("Capturing Image...");
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    Serial.println("Sending Image to FastAPI Server...");
    
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    int httpResponseCode = http.sendRequest("POST", fb->buf, fb->len);

    if (httpResponseCode > 0) {
        Serial.printf("Server Response: %d\n", httpResponseCode);
    } else {
        Serial.printf("Error sending image. HTTP Response Code: %d\n", httpResponseCode);
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    esp_camera_fb_return(fb);
    http.end();
}

// ===========================
// Web Server HTML Page
// ===========================
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM</title>
    <style>
        body { font-family: Arial; text-align: center; background-color: #222; color: white; }
        img { width: 100%; max-width: 640px; }
        button { padding: 10px 20px; font-size: 20px; margin-top: 10px; }
    </style>
</head>
<body>
    <h2>ESP32-CAM Live Stream</h2>
    <img src="/stream" id="video_feed">
    <br>
    <button onclick="capture()">Capture & Send</button>
    <script>
        function capture() {
            fetch('/capture').then(response => alert('Image Captured and Sent!'));
        }
    </script>
</body>
</html>
)rawliteral";

// ===========================
// Camera Web Server Routes
// ===========================
void startCameraServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", htmlPage);
    });

    server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "multipart/x-mixed-replace; boundary=frame", (const uint8_t*)startCameraServer, 0);
    });

    server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request){
        sendCapturedImage();
        request->send(200, "text/plain", "Image Captured and Sent");
    });

    server.begin();
    Serial.println("Web server started, access at: http://<ESP32-IP>/");
}

// ===========================
// ESP32-CAM Setup
// ===========================
void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting ESP32-CAM...");\
    Serial.print("Connected to SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());


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
    config.frame_size = FRAMESIZE_QVGA;
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
    // Connect to Eduroam with WPA2-Enterprise
    // ===========================
    Serial.print("Connecting to: ");
    Serial.println(SSID);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    // Set static IP configuration
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("Failed to configure Static IP! Reverting to DHCP.");
    }

    WiFi.begin(SSID, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD);
    
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempt++;
        if (attempt > 20) {
            Serial.println("\nWiFi connection failed! Check credentials and network settings.");
            return;
        }
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start Web Server for Live Feed & Capture Button
    startCameraServer();
}

// ===========================
// Main Loop
// ===========================
void loop() {
    // No automatic image capture
    delay(10000);
}
