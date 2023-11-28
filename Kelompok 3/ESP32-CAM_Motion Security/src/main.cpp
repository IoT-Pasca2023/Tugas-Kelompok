#define CAMERA_MODEL_AI_THINKER
#include "esp_camera.h"
#include "camera_pins.h"
#include "WiFi.h"
#include "AsyncTCP.h"
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

WiFiClient client;

String WiFiSSID = "Haykull";
String Password = "asd12321";

AsyncWebServer SERVER(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/event");

String serverName = "server-alga.my.id";
String serverPath = "/MotionSecurity/upload_img.php";
const int serverPort = 80;

int status = WL_IDLE_STATUS;

camera_config_t config;
camera_fb_t *fb = NULL;

const int Interval = 30000;
unsigned long previousMillis = 0;
boolean captureRequested = false;

void ConnectWiFi()
{
  Serial.println("Connecting To Wifi");
  WiFi.begin(WiFiSSID.c_str(), Password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("  Wifi Connected");
  IPAddress IP = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(IP);
}

void HandlerCaptureImage(AsyncWebServerRequest *request)
{
  fb = esp_camera_fb_get();
  if (client.connect(serverName.c_str(), serverPort))
  {
    Serial.println("Connected to server");
    String boundary = "------------------------" + String(millis());
    String postData = "";
    postData += "--" + boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"imageFile\"; filename=\"esp32Photo.jpg\"\r\n";
    postData += "Content-Type: image/jpeg\r\n\r\n";

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println("Content-Length: " + String(postData.length() + fb->len + 2));
    client.println();

    client.print(postData);
    client.write(fb->buf, fb->len);
    client.println("\r\n--" + boundary + "--");

    delay(1000);

    esp_camera_fb_return(fb);

    String response = client.readString();
    Serial.println(response);

    // Tutup koneksi
    client.stop();
  }
  else
  {
    request->send(500, "text/plain", "Failed to connect to server");
  }

  if (!fb)
  {
    request->send(500, "text/plain", "Camera capture failed");
    return;
  }
}

void setup()
{
  Serial.begin(9600);
  ConnectWiFi();

  // Camera Initialization
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
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
    Serial.printf("Camera initialization failed with error 0x%x", err);
  {
    return;
  }

  SERVER.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request)
            { HandlerCaptureImage(request); });

  SERVER.begin();
}

void loop()
{
}
