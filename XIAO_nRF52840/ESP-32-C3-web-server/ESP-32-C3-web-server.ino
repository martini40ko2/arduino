#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

//Dziala tylko po wybraniu plytki "SparkFun Pro Micro - ESP32C3"
const char* ssid = "******";
const char* password = "*******";

const int LED_PIN = 8;

WebServer server(80);

String htmlPage(bool ledState) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 LED</title>";
  html += "</head><body style='font-family:Arial; text-align:center;'>";
  html += "<h1>ESP32 Web Server</h1>";

  html += "<p>LED jest ";
  html += ledState ? "<b>WYLACZONA</b></p>" : "<b>WLACZONA</b></p>";

  html += "<a href='/on'><button style='font-size:20px;'>WLACZ</button></a>";
  html += "&nbsp;";
  html += "<a href='/off'><button style='font-size:20px;'>WYLACZ</button></a>";

  html += "</body></html>";
  return html;
}

void handleRoot() {
  bool ledState = digitalRead(LED_PIN);
  server.send(200, "text/html", htmlPage(ledState));
}

void handleOn() {
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Starting connection...");

  pinMode(LED_PIN, OUTPUT);
  delay(100);

  digitalWrite(LED_PIN, HIGH);

  WiFi.begin(ssid, password);
  Serial.print("Laczenie z WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Polaczono! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();
  Serial.println("Serwer WWW uruchomiony");
}

void loop() {
  server.handleClient();
}
