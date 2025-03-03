#include<Arduino.h>
#include <WiFiS3.h>
#include <ArduinoMqttClient.h>
#include<Arduino_Json.h>
#include<info.h>

void connectWiFi();
void sendDataToCloud();
void onMqttMessage(int messageSize);

// OneNet平台MQTT主题
String reply_topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/post/reply";
String set_topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/set";
String get_topic = 	String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/get";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

bool led_status = false;
int obj_num = 0;

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
  }

  Serial.println("已连接到OneNet平台!");

  // 订阅主题
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(reply_topic);
  mqttClient.subscribe(set_topic);
  mqttClient.subscribe(get_topic);
}

void loop() {
  mqttClient.poll();

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
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

void onMqttMessage(int messageSize) {
  //获取到消息，打印消息主题和消息内容
  Serial.println("Received a message with topic '");
  String topic = mqttClient.messageTopic();
  Serial.print(topic);
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  //获取事件主题
  String mqttMessage = "";
  while (mqttClient.available()) {
    mqttMessage += (char)mqttClient.read();
  }
  Serial.println(mqttMessage);

  //设备属性设置
  if(topic == set_topic) {
    JSONVar mqttJson = JSON.parse(mqttMessage);
    if (JSON.typeof(mqttJson) == "undefined") {
      Serial.println("Json parse ERROR");
      return;
    }
    String id = mqttJson["id"];
    bool door = mqttJson["params"]["door"];
    Serial.print("Message id: ");
    Serial.println(id);
    Serial.print("DOOR: ");
    Serial.println(door);

    digitalWrite(LED_BUILTIN, door);
    led_status = door;

    String topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/set_reply";
    JSONVar mqttJsonReply;
    mqttJsonReply["id"] = id;
    mqttJsonReply["code"] = 200;
    mqttJsonReply["msg"] = "success";
    mqttClient.beginMessage(topic);
    mqttClient.print(mqttJsonReply);
    mqttClient.endMessage();
    
    Serial.println("数据已发送：" + mqttJsonReply);
  }

  //设备属性获取
  if (topic == get_topic) {
    JSONVar mqttJson = JSON.parse(mqttMessage);
    if (JSON.typeof(mqttJson) == "undefined") {
      Serial.println("Json parse ERROR");
      return;
    }
    
    String id = mqttJson["id"];
    JSONVar params = mqttJson["params"];
    JSONVar mqttJsonReply;
    mqttJsonReply["id"] = id;
    mqttJsonReply["code"] = 200;

    JSONVar data;
    for (int i = 0; i < params.length(); i ++) {
      String key = params[i];
      if (key == "door") {
        data[key] = led_status;
      }
      if (key == "num") {
        data[key] = random(20);
      }
    }
    mqttJsonReply["data"] = data;
    String topic = String("$sys/") + PRODUCT_ID + "/" + DEVICE_ID + "/thing/property/get_reply";
    mqttClient.beginMessage(topic);
    mqttClient.print(mqttJsonReply);
    mqttClient.endMessage();
  }

  Serial.println();
}