#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const int LDR_PIN = 34;
const int LIGHT_PIN = 18;
const int DARK_THRESHOLD = 1500;  // Adjust after checking LDR readings

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

WebServer server(80);

int lightLevel = 0;
bool isDark = false;
bool automaticMode = true;
bool manualLightOn = false;
bool lightIsOn = false;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Smart Room Light</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #eef4f2;
      color: #18332d;
      text-align: center;
      margin: 0;
      padding: 24px;
    }
    .card {
      max-width: 460px;
      margin: 20px auto;
      padding: 24px;
      background: white;
      border-radius: 18px;
      box-shadow: 0 4px 18px #0002;
    }
    h1 { margin-top: 0; }
    .reading { font-size: 2rem; font-weight: bold; }
    button {
      margin: 7px;
      padding: 12px 18px;
      border: 0;
      border-radius: 10px;
      background: #16855b;
      color: white;
      font-size: 1rem;
      cursor: pointer;
    }
    button:disabled { background: #aaa; cursor: not-allowed; }
    .status { font-weight: bold; }
  </style>
</head>
<body>
  <div class="card">
    <h1>Smart Room Light</h1>

    <p>Light level</p>
    <div class="reading" id="ldr">--</div>

    <p>Room condition: <span class="status" id="condition">--</span></p>
    <p>Mode: <span class="status" id="mode">--</span></p>
    <p>Room light: <span class="status" id="light">--</span></p>

    <h3>Control mode</h3>
    <button onclick="sendCommand('/mode/auto')">AUTO</button>
    <button onclick="sendCommand('/mode/manual')">MANUAL</button>

    <h3>Manual light control</h3>
    <button id="onButton" onclick="sendCommand('/light/on')">TURN ON</button>
    <button id="offButton" onclick="sendCommand('/light/off')">TURN OFF</button>
  </div>

  <script>
    async function sendCommand(path) {
      await fetch(path);
      updateStatus();
    }

    async function updateStatus() {
      try {
        const response = await fetch('/status');
        const data = await response.json();

        document.getElementById('ldr').textContent = data.ldr;
        document.getElementById('condition').textContent = data.condition;
        document.getElementById('mode').textContent = data.mode;
        document.getElementById('light').textContent = data.light;

        document.getElementById('onButton').disabled = data.mode !== 'MANUAL';
        document.getElementById('offButton').disabled = data.mode !== 'MANUAL';
      } catch (error) {
        document.getElementById('condition').textContent = 'Connecting...';
      }
    }

    updateStatus();
    setInterval(updateStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

void updateLight() {
  isDark = lightLevel < DARK_THRESHOLD;

  if (automaticMode) {
    lightIsOn = isDark;
  } else {
    lightIsOn = manualLightOn;
  }

  digitalWrite(LIGHT_PIN, lightIsOn ? HIGH : LOW);
}

void handleStatus() {
  String json = "{";
  json += "\"ldr\":" + String(lightLevel) + ",";
  json += "\"condition\":\"" + String(isDark ? "DARK" : "BRIGHT") + "\",";
  json += "\"mode\":\"" + String(automaticMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"light\":\"" + String(lightIsOn ? "ON" : "OFF") + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);
  analogReadResolution(12);

  lightLevel = analogRead(LDR_PIN);
  updateLight();

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/status", HTTP_GET, handleStatus);

  server.on("/mode/auto", HTTP_GET, []() {
    automaticMode = true;
    updateLight();
    server.send(200, "text/plain", "AUTO mode");
  });

  server.on("/mode/manual", HTTP_GET, []() {
    automaticMode = false;
    updateLight();
    server.send(200, "text/plain", "MANUAL mode");
  });

  server.on("/light/on", HTTP_GET, []() {
    if (!automaticMode) {
      manualLightOn = true;
      updateLight();
    }
    server.send(200, "text/plain", "Light ON command received");
  });

  server.on("/light/off", HTTP_GET, []() {
    if (!automaticMode) {
      manualLightOn = false;
      updateLight();
    }
    server.send(200, "text/plain", "Light OFF command received");
  });

  server.begin();
  Serial.println("Web server started.");
  Serial.println("Open http://localhost:8180 in your browser.");
}

void loop() {
  server.handleClient();

  static unsigned long lastSensorRead = 0;
  static unsigned long lastSerialPrint = 0;
  const unsigned long now = millis();

  if (now - lastSensorRead >= 250) {
    lastSensorRead = now;
    lightLevel = analogRead(LDR_PIN);
    updateLight();
  }

  if (now - lastSerialPrint >= 1000) {
    lastSerialPrint = now;

    Serial.print("LDR: ");
    Serial.print(lightLevel);
    Serial.print(" | Condition: ");
    Serial.print(isDark ? "DARK" : "BRIGHT");
    Serial.print(" | Mode: ");
    Serial.print(automaticMode ? "AUTO" : "MANUAL");
    Serial.print(" | Light: ");
    Serial.println(lightIsOn ? "ON" : "OFF");
  }
}