#include<Arduino.h>
#include <WiFiS3.h>
#include <ArduinoMqttClient.h>

void connectWiFi();
void sendDataToCloud();
void onMqttMessage(int messageSize);

// WiFi配置
// const char* ssid = "USER_1919810";
// const char* password = "Ba+2Na=BaNaNa";
const char* ssid = "2402";
const char* password = "heyanhua2402";

// OneNet MQTT配置
const char* mqttServer = "mqtts.heclouds.com";
int mqttPort = 1883;  // 非SSL端口

// 设备鉴权信息（替换为你的实际信息）
const char* PRODUCT_ID = "fhi0NS1Nvw";
const char* DEVICE_ID = "dev1";
const char* API_KEY = "version=2018-10-31&res=products%2Ffhi0NS1Nvw%2Fdevices%2Fdev1&et=2054535740&method=md5&sign=LM3uKKGDp36w9J5B%2BFVZyw%3D%3D";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // 连接WiFi
  connectWiFi();

  // 设置MQTT客户端
  mqttClient.setId(DEVICE_ID);
  mqttClient.setUsernamePassword(PRODUCT_ID, API_KEY);

  Serial.print("尝试连接MQTT服务器...");
  
  while (!mqttClient.connect(mqttServer, mqttPort)) {
    Serial.print("MQTT连接失败!错误代码: ");
    Serial.println(mqttClient.connectError());
    //while (1);
  }

  Serial.println("已连接到OneNet平台!");

  // 订阅主题（如果需要）
  mqttClient.onMessage(onMqttMessage);
  String topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/post/reply";
  mqttClient.subscribe(topic);
}

void loop() {
  mqttClient.poll();

  // 示例：每5秒发送一条数据
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    sendDataToCloud();
    lastSend = millis();
  }
}

void connectWiFi() {
  Serial.print("正在连接WiFi ");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi已连接");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());
}

void sendDataToCloud() {
  // 构建主题路径
  String topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/post";
  
  // 构建JSON数据
  String payload = "{\"id\":\"123\", \"version\":\"1.0\", \"params\":{\"led\":{\"value\":";
  payload += random(0, 2) ? "false" : "true";  // 生成随机温度值
  payload += "}}}";

  // 发布消息
  mqttClient.beginMessage(topic);
  mqttClient.print(payload);
  mqttClient.endMessage();

  Serial.println("数据已发送：" + payload);
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}