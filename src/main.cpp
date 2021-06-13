#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#ifndef STRUCT_TREE
#define STRUCT_TREE
// #include <relay_model.cpp>
#endif

#include <FS.h>
#include <LITTLEFS.h>

#define FileFS LittleFS

// ********************************* 继电器部分 *********************************
#include <Arduino.h>
#include "gpio.h"

#include <user_interface.h>

// void espSleep(uint64_t seconds)
// {
//     uint64_t USEC = 1000000;
// #ifdef ESP32
//     Serial.printf("Going to sleep %llu seconds\n", seconds);
//     esp_sleep_enable_timer_wakeup(seconds * USEC);
//     esp_deep_sleep_start();
// #elif ESP8266
//     Serial.printf("Going to sleep %llu s. Waking up only if D0 is connected to RST \n", seconds);
//     ESP.deepSleep(seconds * USEC); // 3600e6 = 1 hour in seconds / ESP.deepSleepMax()
// #endif
// }

// BTN DEF
#define BTN 0
#define LED 2

// STATUS DEF
#define Close 1
#define Open 0

/*
var BTNTimer = 0;
function powerBtn(delay) {
  if (delay > 0) {
    console.log(`power press. delay: ${delay}`);
    clearTimeout(BTNTimer);
    digitalWrite(BTN, Open);
    digitalWrite(LED, 1);
    BTNTimer = setTimeout(() => {
      digitalWrite(BTN, Close);
      digitalWrite(LED, 0);
    }, delay);
  }
}

// service
function handle(msg) {
  if (msg == 'short') {
    powerBtn(shortDelay);
  } else if (msg == 'long') {
    powerBtn(6000);
  } else {
    var delay = Number(msg);
    powerBtn(delay);
  }
}
*/

static uint8 RELAY_IF_EXEC = 0;

static os_timer_t os_timer;

void _open_btn()
{
  RELAY_IF_EXEC = 1;
  Serial.println("step1: Open");
  digitalWrite(BTN, Open);
  digitalWrite(LED, 1);
}

void _close_btn()
{
  digitalWrite(BTN, Close);
  digitalWrite(LED, 0);
  Serial.println("step3: Close");
  RELAY_IF_EXEC = 0;
}

void power_btn(unsigned long delay)
{
  if (RELAY_IF_EXEC)
  {
    return;
  }
  Serial.printf("power press. delay: %d \r\n", delay);
  Serial.println("step0: Clear timer");
  os_timer_disarm(&os_timer);

  // 开启
  _open_btn();

  // 设置定时，一段时间后关闭
  Serial.println("step2: Sleep");
  os_timer_setfn(&os_timer, (ETSTimerFunc *)(_close_btn), NULL);
  os_timer_arm(&os_timer, delay, false);
}

// ********************************* 继电器部分END *********************************

// ********************************* MQTT部分 *********************************

const String CONFIG_FILENAME = "mqtt_conf.dat";

struct MQTT_CONFIG
{
  char url[64];
  char username[32];
  char password[32];
  char subscribe_topic[128];
};

MQTT_CONFIG config;

void saveConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  Serial.println(F("SaveMqttCfgFile"));

  if (file)
  {
    file.write((uint8_t *)&config, sizeof(config));

    file.close();
    Serial.println(F("OK"));
  }
  else
  {
    Serial.println(F("failed"));
  }
}

void readConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  Serial.println(F("ReadMqttCfgFile "));

  if (file)
  {
    file.read((uint8_t *)&config, sizeof(config));

    file.close();
    Serial.printf("Read OK. url: %s, topic: %s \r\n", config.url, config.subscribe_topic);
  }
  else
  {
    Serial.println(F("failed. file open error! create empty config file."));
    MQTT_CONFIG *new_config = (struct MQTT_CONFIG *)malloc(sizeof(MQTT_CONFIG));
    config = *new_config;
    saveConfigData();
  }
}

void init_mqtt_config()
{
  // Initialize LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  readConfigData();
}

// ********************************* MQTT部分END *********************************

// ********************************* 配网部分END *********************************

void smartconfig_start()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig等待连接");
  delay(500);
  // 等待配网
  WiFi.beginSmartConfig();

  while (1)
  {
    Serial.print(".");
    delay(500);
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true); // 设置自动连接
      break;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// // Replaces placeholder with LED state value
// String processor(const String &var)
// {
//   Serial.println(var);
//   if (var == "STATE")
//   {
//   }
//   else if (var == "TEMPERATURE")
//   {
//     return getTemperature();
//   }
// }

station_config s_staconf;

void printWifiData()
{
  Serial.print("当前工作模式:"); // 告知用户设备当前工作模式
  Serial.println(WiFi.getMode());
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  Serial.print("MAC address: ");
  Serial.println(buf);

  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.println(rssi);
}

void connect_wifi()
{
  wifi_set_opmode(STATION_MODE);
  WiFi.setAutoConnect(true);

  wifi_station_get_config_default(&s_staconf);
  int count = 0;
  if (s_staconf.ssid && s_staconf.ssid[0])
  {
    WiFi.begin();
    Serial.printf("Connecting to WiFi. ssid: %s, password: %s ", s_staconf.ssid, s_staconf.password);
    while (WiFi.status() != WL_CONNECTED && count++ < 30)
    {
      Serial.printf(".");
      delay(1000);
    }
    Serial.printf("Connected WiFi. status: %d\r\n", WiFi.status());
  }
  if (!(s_staconf.ssid && s_staconf.ssid[0]) || count > 30)
  {
    Serial.println("Enter SmartConfig");
    smartconfig_start();
  }

  // Print ESP32 Local IP Address
  Serial.println("Wifi Connected.");
  delay(500);
  printWifiData();
}

// ********************************* 配网部分END *********************************

AsyncWebServer server(80);

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  // pin init
  _close_btn();
  pinMode(BTN, OUTPUT);
  pinMode(LED, OUTPUT);

  Serial.printf("\r\nStartup. \r\n");

  // Connect to Wi-Fi
  connect_wifi();

  // read mqtt config file
  init_mqtt_config();

  // Route for root / web page
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.serveStatic("/images/", LittleFS, "/images/");
  server.serveStatic("/css/", LittleFS, "/css/");
  server.serveStatic("/scripts/", LittleFS, "/scripts/");

  // Route to set GPIO to HIGH
  server.on("/power_btn", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              //List all parameters
              AsyncWebParameter *delayParam = request->getParam("delay");
              Serial.printf("[GET] /power_btn delay: %d \r\n", delayParam->value().c_str());
              unsigned long delay = strtoul(delayParam->value().c_str(), NULL, 0);
              if (delay < 0 || delay > 60 * 1000)
              {
                char *body = (char *)malloc(24);
                sprintf(body, "delay error. %lu", delay);
                request->send_P(500, "text/plain", body);
              }
              power_btn(delay);
              request->send_P(200, "text/plain", "success");
            });

  // Start server
  server.begin();
  Serial.printf("HTTP Server started. port: 80. IP: %s \r\n", WiFi.localIP().toString().c_str());
}

void loop()
{
  delay(200);
}
