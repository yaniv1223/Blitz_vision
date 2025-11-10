#include "esp_camera.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Camera configuration
#define CAMERA_MODEL_WROVER_KIT
#include "camera_pins.h"

// WiFi credentials
const char* ssid = "Yy";
const char* password = "yaniv123";

// File name settings
const char* photoPrefix = "/photo_";
int photoNumber = 1;

// Button
#define BUTTON_PIN 33

// Web server
WebServer server(80);
#define STREAM_BOUNDARY "frame"

// Prototypes
void sendImageToFlask(uint8_t *image_data, size_t image_len);

// void handle_jpg_stream() {
//   WiFiClient client = server.client();
//   String response = "HTTP/1.1 200 OK\r\n"
//                     "Content-Type: multipart/x-mixed-replace; boundary=" STREAM_BOUNDARY "\r\n\r\n";
//   server.sendContent(response);

//   while (true) {
//     camera_fb_t *fb = esp_camera_fb_get();
//     if (!fb) {
//       Serial.println("Camera capture failed");
//       return;
//     }

//     response = "--" STREAM_BOUNDARY "\r\n"
//                "Content-Type: image/jpeg\r\n"
//                "Content-Length: " + String(fb->len) + "\r\n\r\n";
//     server.sendContent(response);
//     client.write(fb->buf, fb->len);
//     server.sendContent("\r\n");
//     esp_camera_fb_return(fb);

//     if (!client.connected()) break;
//     delay(30);
//   }
// }

void handle_root() {
  String html = "<html><head><title>ESP32-CAM Stream</title></head><body>"
                "<h1>ESP32-CAM Video Stream</h1>"
                "<img src=\"/stream\" width=\"320\" height=\"240\" />"
                "<p>Press the button to take a photo and save it to SD card.</p>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void sendImageToFlask(uint8_t *image_data, size_t image_len) {
  WiFiClient client;
  client.setTimeout(30000);  // Timeout

  HTTPClient http;
  http.setTimeout(30000);
  http.useHTTP10();
  http.setReuse(false);
  http.begin(client, "http://10.87.61.245:5000/upload");
  http.addHeader("Content-Type", "application/octet-stream");
  http.addHeader("Connection", "close");

  int httpCode = http.POST(image_data, image_len);
  if (httpCode == HTTP_CODE_OK) {
    Serial.println(" Receiving audio from server…");
    String audioFileName = "/audio_" + String(photoNumber) + ".mp3";
    File audioFile = SD_MMC.open(audioFileName.c_str(), FILE_WRITE);
    if (!audioFile) {
      Serial.println(" Failed to open audio file");
      http.end();
      return;
    }

    WiFiClient *stream = http.getStreamPtr();
    uint8_t buffer[512];
    size_t total = 0;

    while (stream->connected() && stream->available()) {
      size_t n = stream->readBytes(buffer, sizeof(buffer));
      if (n) {
        audioFile.write(buffer, n);
        total += n;
      }
    }
    audioFile.close();
    Serial.printf(" Audio saved: %s (%d bytes)\n", audioFileName.c_str(), total);
  } else {
    Serial.printf(" HTTP POST failed: %d\n", httpCode);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(BUTTON_PIN, INPUT_PULLUP);

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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println(" Camera init failed");
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_QVGA);

  Serial.println("Initializing SD card...");
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println(" Failed to initialize SD card!");
    return;
  }

  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println(" No SD card attached");
    return;
  }

  Serial.println(" SD card initialized.");

  server.on("/", handle_root);
  server.on("/stream", HTTP_GET, handle_jpg_stream);
  server.begin();
  Serial.println(" HTTP server started");
}

void loop() {
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println(" Button pressed - taking photo...");
    delay(200);  // Debounce

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println(" Camera capture failed");
      return;
    }

    String photoFileName = photoPrefix + String(photoNumber) + ".jpg";

    fs::FS &fs = SD_MMC;
    Serial.printf(" Saving photo: %s\n", photoFileName.c_str());

    File photoFile = fs.open(photoFileName.c_str(), FILE_WRITE);
    if (!photoFile) {
      Serial.println(" Failed to open photo file");
    } else {
      photoFile.write(fb->buf, fb->len);
      photoFile.close();
      Serial.println(" Photo saved to SD card");
    }

    sendImageToFlask(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    photoNumber++;

    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
  }

  delay(10);
}
