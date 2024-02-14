/*
 AsyncElegantOTA Demo Example - This example will work for both ESP8266 & ESP32 microcontrollers.
 -----
 Author: Ayush Sharma ( https://github.com/ayushsharma82 )

 Important Notice: Star the repository on Github if you like the library! :)
 Repository Link: https://github.com/ayushsharma82/AsyncElegantOTA
*/

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#endif

#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "chaynik_webpage.h"
#include "chaynik_js.h"

const char *ssid = "*********";
const char *password = "*********";

const char *host = "chaynik";
const int onoffPin = D1;
const int varPin = D2;
// const int statusPin = 4;
boolean status = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *wsClient;

void notifyClients(boolean value)
{
  Serial.println("Notifying clients...");
  String message;
  message += String(value);

  Serial.print("status: ");
  Serial.println(message);
  ws.textAll(message);
}

boolean getStatus()
{
  // return digitalRead(statusPin) ? "on" : "off";
  return status;
}

void toggleStatus()
{
  // return digitalRead(statusPin) ? "on" : "off";
  status = !status;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    Serial.println((char *)data);
    data[len] = 0;
    if (strcmp((char *)data, "onoff") == 0)
    {
      pinMode(onoffPin, OUTPUT);
      delay(20);
      Serial.print("digitalWrite(");
      Serial.print(onoffPin);
      Serial.print(", ");
      Serial.print("HIGH");
      Serial.println(")");
      digitalWrite(onoffPin, LOW);
      digitalWrite(2, LOW);
      delay(80);
      pinMode(onoffPin, INPUT);
      // digitalWrite(onoffPin, HIGH);
      // digitalWrite(2, HIGH);
      toggleStatus();
    }
    if (strcmp((char *)data, "var") == 0)
    {
      pinMode(varPin, OUTPUT);
      delay(20);
      Serial.print("digitalWrite(");
      Serial.print(varPin);
      Serial.print(", ");
      Serial.print("LOW");
      Serial.println(")");
      digitalWrite(varPin, LOW);
      digitalWrite(2, LOW);
      delay(80);
      pinMode(varPin, INPUT);
      // digitalWrite(varPin, HIGH);
      // digitalWrite(2, HIGH);
      status = true;
    }

    // Serial.print("status");
    // Serial.println(getStatus());
    notifyClients(getStatus());
  }
  else
  {
    Serial.println("Invalid command");
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    notifyClients(getStatus());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

void setup(void)
{
  Serial.begin(115200);
  // pinMode(onoffPin, OUTPUT);
  // pinMode(varPin, OUTPUT);
  // digitalWrite(onoffPin, HIGH);
  // digitalWrite(varPin, HIGH);
  // pinMode(statusPin, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.hostname(host);

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host))
  {
    Serial.println("Error setting up MDNS responder!");
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

    while (1)
    {
      delay(1000);
    }
  }
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS responder started");

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // respond with the compressed frontend
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", chaynik_HTML, sizeof(chaynik_HTML));
    response->addHeader("Content-Encoding","gzip");
    response->addHeader("Cache-Control","public, max-age=900");
    request->send(response); });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // respond with the compressed frontend
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", chaynik_JS, sizeof(chaynik_JS));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "public, max-age=900");
    request->send(response); });

  AsyncElegantOTA.begin(&server); // Start AsyncElegantOTA
  server.onNotFound(notFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
#ifdef ESP8266
  MDNS.update();
#endif
}
