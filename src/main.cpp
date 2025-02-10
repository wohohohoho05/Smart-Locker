#include<Arduino.h>
#include <WiFiS3.h>
#include <ArduinoMqttClient.h>
#include<Arduino_Json.h>
#include<info.h>

void connectWiFi();
void sendDataToCloud();
void onMqttMessage(int messageSize);

String reply_topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/post/reply";
String set_topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/set";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void setup() {
  
  Serial.begin(9600);
  while (!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
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
  mqttClient.subscribe(reply_topic);
  mqttClient.subscribe(set_topic);
}

void loop() {
  mqttClient.poll();

  // 示例：每5秒发送一条数据
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    // sendDataToCloud();
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
  payload += random(0, 2) ? "false" : "true";  // 生成随机01
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
  String topic = mqttClient.messageTopic();
  Serial.print(topic);
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  String mqttMessage = "";
  while (mqttClient.available()) {
    mqttMessage += (char)mqttClient.read();
  }
  Serial.println(mqttMessage);

  if(topic == set_topic) {
    JSONVar mqttJson = JSON.parse(mqttMessage);
    if (JSON.typeof(mqttJson) == "undefined") {
      Serial.println("Json parse ERROR");
      return;
    }
    String id = mqttJson["id"];
    bool led = mqttJson["params"]["led"];
    Serial.print("Message id: ");
    Serial.println(id);
    Serial.print("LED: ");
    Serial.println(led);

    digitalWrite(LED_BUILTIN, led);

    String topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/set_reply";
    String payload = "{\"id\":\"";
    payload += id;
    payload += "\", \"code\":200, \"msg\":\"success\"}";
    mqttClient.beginMessage(topic);
    mqttClient.print(payload);
    mqttClient.endMessage();
    
    Serial.println("数据已发送：" + payload);
  }

  Serial.println();
}