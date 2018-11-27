#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>

#include "settings.h"

ESP8266WiFiMulti wifiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

File fsUploadFile;

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.begin(115200);
  delay(10);
  Serial.println("");

  startWiFi();
  startSPIFFS();
  startWebSocketServer();
  startMDNS();
  startWebServer();
}

unsigned long prevMillis = millis();
unsigned int animationDelay = 32;
unsigned int rgb[3] = {0, 0, 0};
bool fade = false;
int hue = 0;

void loop() {
  webSocket.loop();
  server.handleClient();

  if (fade) {
    if (millis() > prevMillis + animationDelay) {
      if (++hue == 360) hue = 0;
      setHue(hue);
      prevMillis = millis();
    }
  }
}

void startWiFi() {
  WiFi.softAP(ap_ssid, ap_pass);
  Serial.print("Access point \"");
  Serial.print(ap_ssid);
  Serial.println("\" started\n");

  wifiMulti.addAP(wifi_ssid, wifi_pass);

  Serial.print("Connecting ");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {
    setRGB(0, 0, 511);
    delay(125);
    setRGB(0, 0, 255);
    delay(125);
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
    fade = true;
  } else {
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println();
}

void startSPIFFS() {
  SPIFFS.begin();
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.println();
  }
}

void startWebSocketServer() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");
}

void startMDNS() {
  MDNS.begin(esp_name);
  Serial.print("mDNS responder started: http://");
  Serial.print(esp_name);
  Serial.println(".local/");
}

void startWebServer() {
  server.on("/edit.html", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

void handleNotFound() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "404: File not found");
  }
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile not found: ") + path);
  return false;
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {
      String pathWithGz = path + ".gz";
      if (SPIFFS.exists(pathWithGz)) SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload file: ");
    Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      Serial.print("handleFileUpload size: ");
      Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: Couldn't create file");
    }
  }
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
      if (payload[0] == '#') {
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
        int r = ((rgb >> 20) & 0x3FF);
        int g = ((rgb >> 10) & 0x3FF);
        int b = rgb & 0x3FF;
        fade = false;
        // rgb[0] = r;
        // rgb[1] = g;
        // rgb[2] = b;
        setRGB(r, g, b);
      } else if (payload[0] == 'd') {
        int delay = (uint32_t) strtol((const char *) &payload[1], NULL, 10);
        if (delay > 500) delay = 500;
        if (delay < 0) delay = 0;
        animationDelay = delay;
      } else if (payload[0] == 'R') {
        fade = true;
      } else if (payload[0] == 'N') {
        fade = false;
      }
      break;
  }
}

// ================================================== Helper Functions ==================================================

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

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KiB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MiB";
  }
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".webmanifest")) return "application/json";
  else if (filename.endsWith(".xml")) return "application/xml";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void setHue(int hue) {
  hue %= 360;
  float radH = hue * 3.142 / 180;
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