#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ArduinoOTA.h>

#include "settings.bed.h"

ESP8266WiFiMulti wifiMulti;

WebSocketsServer webSocket(81);

const float pi = 3.14159265359;

unsigned long prevMillis = millis();
unsigned int animationDelay = 50;
unsigned short currentAnimation = 0;
int rgb[] = {1023, 1023, 1023};
float hue = 0.0;
float brightnessY = 0.0;
float brightnessTime = 0.0;
float brightnessTimeAdd = 0.03;
bool blink = true;

void startWiFi();
void startWebSocketServer();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);
void animation();
void setRGB(int r, int g, int b);
void setHue(float hue);
String rgbToHex(int color[]);

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.begin(115200);
  delay(10);
  Serial.println("");

  startWiFi();

  ArduinoOTA.setHostname(esp_name);
  ArduinoOTA.setPassword(ap_pass);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  startWebSocketServer();
  randomSeed(analogRead(0));
}

void loop() {
  ArduinoOTA.handle();
  webSocket.loop();
  animation();
}

void startWiFi() {
  WiFi.hostname(esp_name);
  WiFi.softAP(ap_ssid, ap_pass, 1, ap_hidden);
  Serial.print("Access point \"");
  Serial.print(ap_ssid);
  Serial.println("\" started\n");

  wifiMulti.addAP(wifi_ssid, wifi_pass);

  Serial.print("Connecting ");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {
    delay(250);
    setRGB(0, 85, 255);
    delay(250);
    setRGB(0, 42, 128);
    Serial.print('.');
  }
  Serial.print("");
  if (WiFi.softAPgetStationNum() == 0) {
    setRGB(0, 512, 0);
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    delay(1000);
    setRGB(0, 0, 0);
    currentAnimation = 2;
  } else {
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println();
}

void startWebSocketServer() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '0') { // off
        currentAnimation = 0;
        setRGB(0, 0, 0);
      } else if (payload[0] == 'g') { // get status
        webSocket.sendTXT(num, "" + rgbToHex(rgb) + "," + String(currentAnimation)  + ","
                                                        + String(animationDelay));
      } else if (payload[0] == '#') {
        uint32_t color = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
        int r = map(((color >> 16) & 0xFF), 0, 255, 0, 1023);
        int g = map(((color >> 8) & 0xFF), 0, 255, 0, 1023);
        int b = map(((color >> 0) & 0xFF), 0, 255, 0, 1023);
        if (currentAnimation <= 2 || currentAnimation == 5) currentAnimation = 1;
        rgb[0] = r;
        rgb[1] = g;
        rgb[2] = b;
        if (currentAnimation != 3 && currentAnimation != 4) setRGB(r, g, b);
      } else if (payload[0] == 'd') {
        int delay = (uint32_t) strtol((const char *) &payload[1], NULL, 10);
        if (delay > 100) delay = 100;
        if (delay < 0) delay = 0;
        animationDelay = delay;
      } else if (payload[0] == 'a') {
        unsigned short newAnimation = (unsigned short) strtol((const char *) &payload[1], NULL, 10);
        currentAnimation = newAnimation;
        if (newAnimation == 5) hue = 0;
      }
      break;
  }
}

// ================================================== Helper Functions ==================================================

void animation() {
  switch (currentAnimation) {
    case 0: // off
    case 1: // no animation
      break;
    case 2: // fade animation
      if (millis() > prevMillis + animationDelay) {
        hue += 0.5;
        if (hue > 360) hue = 0;
        setHue(hue);
        prevMillis = millis();
      }
      break;
    case 3: // blink animation
      if (millis() > prevMillis + animationDelay * 10) {
        if (blink) {
          setRGB(rgb[0], rgb[1], rgb[2]);
        } else {
          setRGB(0, 0, 0);
        }
        blink = !blink;
        prevMillis = millis();
      }
      break;
    case 4: // breathing animation
      if (millis() > prevMillis + animationDelay) {
        int newRgb[] = {0, 0, 0};
        if (brightnessTime >= pi*2) brightnessTime = 0.0;
        brightnessTime += 0.02;
        brightnessY = (cos(brightnessTime) + 1) / 2;
        newRgb[0] = rgb[0] * brightnessY;
        newRgb[1] = rgb[1] * brightnessY;
        newRgb[2] = rgb[2] * brightnessY;
        setRGB(newRgb[0], newRgb[1], newRgb[2]);
        prevMillis = millis();
      }
      break;
    case 5: // fire animation
      if (millis() > prevMillis + map(animationDelay, 0, 100, 0, 15)) {
        float newHue = 0;
        if (brightnessTime >= pi*2) {
          brightnessTime = 0.0;
          brightnessTimeAdd = random(1, 10) / 100.0;
        }
        brightnessTime += brightnessTimeAdd;
        brightnessY = -cos(brightnessTime) + 1;
        float brightnessPercent = brightnessY / 2;
        newHue = 18 * brightnessPercent;
        setHue(newHue);
        prevMillis = millis();
      }
      break;
    default:
      break;
  }
}

void setRGB(int r, int g, int b) {
  if (r > 1023) r = 1023;
  if (r < 0) r = 0;
  if (g > 1023) g = 1023;
  if (g < 0) g = 0;
  if (b > 1023) b = 1023;
  if (b < 0) b = 0;
  
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void setHue(float hue) {
  if (hue >= 360) hue = 0.0;
  float radH = hue * pi / 180;
  float rf, gf, bf;

  if (hue >= 0 && hue < 120) {
    rf = cos(radH * 3 / 4);
    gf = sin(radH * 3 / 4);
    bf = 0;
  } else if (hue >= 120 && hue < 240) {
    radH -= 2.09439;
    gf = cos(radH * 3 / 4);
    bf = sin(radH * 3 / 4);
    rf = 0;
  } else if (hue >= 240 && hue < 360) {
    radH -= 4.188787;
    bf = cos(radH * 3 / 4);
    rf = sin(radH * 3 / 4);
    gf = 0;
  }
  int r = rf * rf * 1023;
  int g = gf * gf * 1023;
  int b = bf * bf * 1023;

  setRGB(r, g, b);
}

String rgbToHex(int color[]) {
  return String(map(color[0], 0, 1023, 0, 255)) + ","
       + String(map(color[1], 0, 1023, 0, 255)) + ","
       + String(map(color[2], 0, 1023, 0, 255));
}