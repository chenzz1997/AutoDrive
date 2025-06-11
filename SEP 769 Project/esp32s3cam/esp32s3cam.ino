#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>

#define RX 3
#define TX 40

// 摄像头引脚配置
#define CAMERA_MODEL_ESP32S3_CAM
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

// Wi-Fi 配置
const char* ssid = "ESP32S3CAM";
const char* password = "";

// 服务器对象
WiFiServer videoServer(80);    // 视频流服务器（端口80）
WiFiServer resultServer(81);   // 接收预测结果的服务器（端口81）

void setup() {
  Serial.begin(9600);
  Serial.setDebugOutput(true);

  // 初始化摄像头
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("摄像头初始化失败: 0x%x", err);
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_hmirror(s, 1);

  // 启动 Wi-Fi 热点
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("热点 IP: ");
  Serial.println(ip);

  videoServer.begin();
  resultServer.begin();       // 启动结果接收服务器

  // 初始化串口
  Serial2.begin(115200, SERIAL_8N1, RX, TX);
}

void loop() {
  
  // 非阻塞方式处理视频流客户端
  handleVideoClient();
}

// 分离视频流处理逻辑
void handleVideoClient() {
  WiFiClient videoClient = videoServer.available();
  if (videoClient) {
    Serial.println("视频客户端连接");
    String response = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    videoClient.print(response);

    while (videoClient.connected()) {
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("捕获失败");
        continue;
      }
      videoClient.print("--frame\r\nContent-Type: image/jpeg\r\n\r\n");
      videoClient.write(fb->buf, fb->len);
      videoClient.print("\r\n");
      esp_camera_fb_return(fb);
      delay(100);

      // 处理结果客户端请求（避免阻塞）
      handleResultClient();
    }
    videoClient.stop();
    Serial.println("视频客户端断开");
  }
}

// 分离结果处理逻辑
void handleResultClient() {
  static WiFiClient resultClient = resultServer.available();

  if (resultClient && resultClient.connected()) {
    if (resultClient.available()) {
      String result = resultClient.readStringUntil('\n');
      Serial2.println(result);
      Serial.print("发送结果: ");
      Serial.println(result);
    }
  } else {
    resultClient = resultServer.available();  // 尝试重新连接
  }
}
