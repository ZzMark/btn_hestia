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
    Serial.printf("Read OK. url: %s, topic: %s \n", config.url, config.subscribe_topic);
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

void connect_wifi()
{
  wifi_set_opmode(STATION_MODE);

  wifi_station_get_config_default(&s_staconf);
  int count = 0;
  if (s_staconf.ssid && s_staconf.ssid[0])
  {
    Serial.printf("Connecting to WiFi. ssid: %s, password: %s ", s_staconf.ssid, s_staconf.password);
    while (WiFi.status() != WL_CONNECTED && count++ < 30)
    {
      Serial.printf(".");
      delay(1000);
    }
    Serial.printf("Connect WiFi error.");
  }
  if (!s_staconf.ssid || count > 30)
  {
    Serial.println("Enter SmartConfig");
    smartconfig_start();
  }

  // Print ESP32 Local IP Address
  Serial.printf("Wifi Connected. IP: %s \n", WiFi.localIP().toString());
}

AsyncWebServer server(80);

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  // pinMode(ledPin, OUTPUT);

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
              AsyncWebParameter *delay = request->getParam("delay");
              Serial.printf("[GET] /power_btn delay: %d", delay->value().c_str());
              // power_btn(100);
              request->send_P(200, "text/plain", "success");
            });

  // Start server
  server.begin();
  Serial.printf("HTTP Server started. port: 80. IP: %s \n", WiFi.localIP().toString());
}

void loop()
{
  delay(200);
}
