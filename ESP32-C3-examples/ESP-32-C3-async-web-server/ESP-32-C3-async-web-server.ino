#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <U8g2lib.h>
#include <Wire.h>

// Replace with your network credentials
const char* ssid = "siec";
const char* password = "haslo";

#define POT_PIN 7
#define LED_PIN 8

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);
int width = 70;
int height = 40;
int xOffset = 30; // = (132-w)/2
int yOffset = 24; // = (64-h)/2

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String potValue = "0";

// HTML + JavaScript 
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Real-Time Dashboard</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    button { padding: 10px 20px; font-size: 16px; }
  </style>
</head>
<body>

<h2>ESP32 Real-Time Dashboard</h2>

<p>Potentiometer Value:</p>
<h1 id="pot">0</h1>

<button onclick="toggleLED()">Toggle LED</button>

<script>
let ws = new WebSocket(`ws://${location.host}/ws`);

ws.onmessage = (event) => {
  document.getElementById("pot").innerHTML = event.data;
};

function toggleLED() {
  ws.send("toggle");
}
</script>

</body>
</html>
)rawliteral";

// Handle WebSocket events
void onWebSocketEvent(AsyncWebSocket *server,
                      AsyncWebSocketClient *client,
                      AwsEventType type,
                      void *arg,
                      uint8_t *data,
                      size_t len) {

  if (type == WS_EVT_DATA) {
    String msg = "";
    for (int i = 0; i < len; i++) {
      msg += (char)data[i];
    }

    if (msg == "toggle") {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
 
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  //setup display
  u8g2.begin();
  u8g2.setContrast(255);    // set contrast to maximum
  u8g2.setBusClock(400000); // 400kHz I2C
  u8g2.setFont(u8g2_font_ncenB08_tr);	 // choose a suitable font
  u8g2.clearBuffer();                                      // clear the internal memory

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("ESP32 IP: ");
  String ipStr = WiFi.localIP().toString();
  Serial.println(ipStr);

  u8g2.setCursor(xOffset + 1, yOffset + 25);
  u8g2.print(ipStr);
  u8g2.sendBuffer(); // transfer internal memory to the display

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
}

void loop() {
  int pot = analogRead(POT_PIN);
  potValue = String(pot);

  ws.textAll(potValue);   // Push value instantly to browser
  delay(100);
}