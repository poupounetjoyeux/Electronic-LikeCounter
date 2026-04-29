#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <PxMatrix.h>
#include "Constants.h"

Ticker display_ticker;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
JsonDocument jsonPayload;
PxMATRIX display(MATRIX_WIDTH, MATRIX_HEIGHT, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

void connectWifi()
{
  delay(10);
  Serial.printf("Connecting to %s\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void connectMqtt()
{
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(DEVICE_NAME))
    {
      Serial.println("MQTT connected");
      mqttClient.subscribe(STATS_TOPIC);
      Serial.println("Topic " STATS_TOPIC " subscribed");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 3 seconds"); // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

void callback(char *topicP, byte *payload, unsigned int length)
{
  String topic(topicP);
  if (topic.equals(STATS_TOPIC))
  {
    Serial.printf("Received a stat update");
    deserializeJson(jsonPayload, payload);
  }
  else
  {
    Serial.print("Received an unknown topic ");
    Serial.println(topic);
  }
}

void drawStartup()
{
  display.clearDisplay();
  display.setTextColor(display.color565(255, 255, 255));

  logoIcon.draw(&display, 16, 0);

  display.setTextSize(1);
  display.setCursor(9, 25);
  display.print("29.06.24");
}

void drawTitle(String title, uint16_t color)
{
  display.setTextColor(color);
  display.setTextSize(1);
  display.setCursor((64 - title.length() * 6) / 2, 2);
  display.print(title);
}

void drawWinner(String title, String winner, uint16_t color)
{
  display.clearDisplay();

  drawTitle(title, color);

  winnerIcon.draw(&display, 8, 12);
  display.setCursor((64 - winner.length() * 6) / 2, 22);
  display.print(winner);
}

void drawCount(String title, Icon icon, uint16_t color, int currentCount)
{
  display.clearDisplay();

  drawTitle(title, color);

  icon.draw(&display, (27 - icon.width) / 2, 13);

  display.setTextSize(2);
  display.setCursor(24, 14);
  display.printf("%.03d", currentCount);
}

void drawPunchCount(int currentCount)
{
  drawCount("Punch", punchIcon, display.color565(255, 128, 0), currentCount);
}

void drawSexCount(int currentCount)
{
  drawCount("Sex/Beach", sexIcon, display.color565(240, 15, 66), currentCount);
}

void drawMessageCount(int currentCount)
{
  drawCount("Repondeur", telIcon, display.color565(255, 255, 255), currentCount);
}

void drawPhotoCount(int currentCount)
{
  drawCount("Photos", photoIcon, display.color565(117, 77, 55), currentCount);
}

void drawPunchWinner(String winner)
{
  drawWinner("Punch", winner, display.color565(255, 128, 0));
}

void drawSexWinner(String winner)
{
  drawWinner("Sex/Beach", winner, display.color565(240, 15, 66));
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }
  Serial.println();

  connectWifi();

  mqttClient.setServer(WiFi.gatewayIP(), MQTT_PORT);
  mqttClient.setCallback(callback);
  connectMqtt();

  mqttClient.publish(INIT_TOPIC, INIT_PAYLOAD, false);

  display.begin(MATRIX_REFRESH);
  drawStartup();
  Serial.println("Device setupped");
}

int turn = -1;
uint32_t prev = millis();
void loop()
{
  if (!mqttClient.connected())
  {
    connectMqtt();
  }
  mqttClient.loop();
  display.display();

  if (jsonPayload.isNull())
  {
    return;
  }

  if (millis() - prev > DISPLAY_DELAY * 1000)
  {
    if (turn == 5)
    {
      turn = 0;
    }
    else
    {
      turn++;
    }

    if (turn == 0 && jsonPayload.containsKey("PunchCount"))
    {
      drawPunchCount(jsonPayload["PunchCount"].as<int>());
      prev = millis();
      return;
    }

    if (turn == 1 && jsonPayload.containsKey("PunchWinner"))
    {
      drawPunchWinner(jsonPayload["PunchWinner"].as<const char *>());
      prev = millis();
      return;
    }

    if (turn == 2 && jsonPayload.containsKey("SexCount"))
    {
      drawSexCount(jsonPayload["SexCount"].as<int>());
      prev = millis();
      return;
    }

    if (turn == 3 && jsonPayload.containsKey("SexWinner"))
    {
      drawSexWinner(jsonPayload["SexWinner"].as<const char *>());
      prev = millis();
      return;
    }
    
    if (turn == 4 && jsonPayload.containsKey("PhotoCount"))
    {
      drawPhotoCount(jsonPayload["PhotoCount"].as<int>());
      prev = millis();
      return;
    }
    
    if (turn == 5 && jsonPayload.containsKey("MessageCount"))
    {
      drawMessageCount(jsonPayload["MessageCount"].as<int>());
      prev = millis();
      return;
    }
  }
}