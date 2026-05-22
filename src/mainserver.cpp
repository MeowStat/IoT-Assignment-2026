#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

bool isAPMode = true;

WebServer mainServer(80);

// NeoPixel (peripheral 2) - single pixel
Adafruit_NeoPixel neo(1, NEO_PIN, NEO_GRB + NEO_KHZ800);

unsigned long connect_start_ms = 0;
bool connecting = false;
unsigned long last_wifi_status_log_ms = 0;

String mainPage() {
  SensorData_t sensorData;
  xQueuePeek(xSensorQueue, &sensorData, 0);
  float temperature = sensorData.temperature;
  float humidity = sensorData.humidity;
  String led1 = led1_state ? "ON" : "OFF";
  String led2 = led2_state ? "ON" : "OFF";

  String ptr = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ESP32 Core | Vineyard Monitor</title>
    <style>
      :root {
        /* Grape & Blueberry Color Palette */
        --theme-color: #b829ea; /* Vibrant Neon Grape */
        --theme-glow: rgba(184, 41, 234, 0.6);
        --theme-glow-intense: rgba(184, 41, 234, 0.95);
        --bg-dark: #0a0514; /* Very deep blueberry/blackberry */
        --bg-surface: rgba(20, 10, 35, 0.75);
        --text-main: #f8fafc;
        --text-dim: #a79fcb;
        --border-color: rgba(184, 41, 234, 0.2);
      }
      * { box-sizing: border-box; margin: 0; padding: 0; }
      body {
        font-family: "Inter", "Segoe UI", system-ui, sans-serif;
        background-color: var(--bg-dark);
        background-image: 
          radial-gradient(circle at 15% 50%, rgba(184, 41, 234, 0.12) 0%, transparent 50%),
          radial-gradient(circle at 85% 30%, rgba(49, 10, 92, 0.8) 0%, var(--bg-dark) 100%);
        color: var(--text-main);
        min-height: 100vh;
        display: flex;
        align-items: center;
        justify-content: center;
        overflow-x: hidden;
      }
      .wrapper { display: flex; justify-content: center; align-items: center; width: 100%; padding: 20px; }
      .app-card {
        background: var(--bg-surface);
        border: 1px solid var(--border-color);
        border-radius: 20px;
        padding: 2.5rem;
        width: 100%;
        max-width: 400px;
        box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.8), inset 0 0 20px rgba(184, 41, 234, 0.05);
        backdrop-filter: blur(16px);
        -webkit-backdrop-filter: blur(16px);
      }
      header { text-align: center; margin-bottom: 2rem; }
      header h1 { font-size: 1.35rem; letter-spacing: 0.05em; color: var(--text-main); font-weight: 700; display: flex; align-items: center; justify-content: center; gap: 10px; text-shadow: 0 0 15px rgba(184, 41, 234, 0.3); }
      
      /* Intensified pulsing glow for the live indicator */
      .live-dot { 
        height: 10px; 
        width: 10px; 
        background-color: var(--theme-color); 
        border-radius: 50%; 
        display: inline-block; 
        box-shadow: 0 0 15px var(--theme-glow-intense), 0 0 30px var(--theme-glow); 
        animation: pulse 2s infinite; 
      }
      
      header p { font-size: 0.85rem; color: var(--text-dim); margin-top: 0.3rem; }
      .stats-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 1rem; margin-bottom: 2rem; }
      .stat-box {
        background: rgba(255, 255, 255, 0.02);
        border: 1px solid var(--border-color);
        padding: 1.25rem;
        border-radius: 16px;
        display: flex;
        flex-direction: column;
        gap: 0.5rem;
        transition: transform 0.3s ease, box-shadow 0.3s ease;
      }
      .stat-box:hover { 
        transform: translateY(-2px); 
        border-color: var(--theme-color);
        box-shadow: 0 10px 20px rgba(0,0,0,0.4), 0 0 15px rgba(184, 41, 234, 0.2);
      }
      .stat-label { display: flex; align-items: center; gap: 6px; font-size: 0.75rem; font-weight: 600; color: var(--text-dim); text-transform: uppercase; letter-spacing: 0.05em; }
      .stat-label svg { width: 14px; height: 14px; fill: currentColor; }
      
      /* Values now glow slightly */
      .stat-value { font-size: 1.75rem; font-weight: 700; color: var(--theme-color); font-variant-numeric: tabular-nums; text-shadow: 0 0 20px var(--theme-glow); }
      .unit { font-size: 1rem; color: var(--text-dim); font-weight: 500; margin-left: 2px; text-shadow: none; }
      
      .control-list { display: flex; flex-direction: column; gap: 0.85rem; margin-bottom: 1.75rem; }
      .btn-toggle {
        background: rgba(255, 255, 255, 0.02);
        border: 1px solid var(--border-color);
        color: var(--text-main);
        padding: 1.1rem 1.25rem;
        border-radius: 14px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        cursor: pointer;
        transition: all 0.25s ease;
        font-weight: 500;
        width: 100%;
      }
      .btn-toggle:hover { border-color: var(--theme-color); background: rgba(184, 41, 234, 0.05); }
      .btn-toggle:active { transform: scale(0.98); }
      
      .badge { font-size: 0.75rem; padding: 5px 12px; border-radius: 8px; font-weight: 700; letter-spacing: 0.05em; transition: all 0.3s ease; }
      
      /* Intensified badge glow */
      .badge-active { 
        background: rgba(184, 41, 234, 0.15); 
        color: var(--text-main); 
        border: 1px solid var(--theme-color); 
        box-shadow: 0 0 20px var(--theme-glow), inset 0 0 10px var(--theme-glow); 
        text-shadow: 0 0 5px #fff;
      }
      .badge-idle { background: rgba(255, 255, 255, 0.05); color: var(--text-dim); border: 1px solid transparent; box-shadow: none; }
      
      .btn-settings {
        width: 100%; background: var(--text-main); color: var(--bg-dark); border: none; padding: 1.1rem;
        border-radius: 14px; font-weight: 700; cursor: pointer; transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1); text-transform: uppercase; letter-spacing: 0.05em; display: flex; align-items: center; justify-content: center; gap: 8px;
      }
      .btn-settings svg { width: 16px; height: 16px; fill: currentColor; }
      
      /* Intensified button hover glow */
      .btn-settings:hover { 
        background: var(--theme-color); 
        box-shadow: 0 0 30px var(--theme-glow-intense); 
        color: #fff; 
        transform: translateY(-2px);
      }
      .btn-settings:active { transform: scale(0.98); box-shadow: 0 0 15px var(--theme-glow); }
      
      /* Expanded pulse animation radius */
      @keyframes pulse {
        0% { box-shadow: 0 0 0 0 var(--theme-glow-intense); }
        70% { box-shadow: 0 0 0 12px rgba(184, 41, 234, 0); }
        100% { box-shadow: 0 0 0 0 rgba(184, 41, 234, 0); }
      }
      
      @media (max-width: 480px) {
        .app-card { padding: 1.5rem; border-radius: 0; border: none; background: transparent; backdrop-filter: none; box-shadow: none; }
      }
    </style>
  </head>
  <body>
    <div class="wrapper">
      <div class="app-card">
        <header>
          <h1><span class="live-dot"></span> VINEYARD CORE</h1>
          <p>Real-time Microclimate Telemetry</p>
        </header>
        <div class="stats-grid">
          <div class="stat-box">
            <span class="stat-label">
              <svg viewBox="0 0 24 24"><path d="M15 13V5A3 3 0 0 0 9 5V13A5 5 0 1 0 15 13M12 4A1 1 0 0 1 13 5V8H11V5A1 1 0 0 1 12 4Z"/></svg>
              Temp
            </span>
            <div class="stat-value"><span id="temp">--</span><span class="unit">°C</span></div>
          </div>
          <div class="stat-box">
            <span class="stat-label">
              <svg viewBox="0 0 24 24"><path d="M12 20A6 6 0 0 1 6 14C6 10 12 3.25 12 3.25S18 10 18 14A6 6 0 0 1 12 20M12 5.5C10.5 7.5 7.5 11.46 7.5 14A4.5 4.5 0 0 0 12 18.5A4.5 4.5 0 0 0 16.5 14C16.5 11.46 13.5 7.5 12 5.5Z"/></svg>
              Humidity
            </span>
            <div class="stat-value"><span id="hum">--</span><span class="unit">%</span></div>
          </div>
        </div>
        <div class="control-list">
          <div class="btn-toggle" onclick="toggleLED(1)">
            Irrigation Valve <span id="l1" class="badge badge-idle">OFF</span>
          </div>
          <div class="btn-toggle" onclick="toggleLED(2)">
            Cooling Fan <span id="l2" class="badge badge-idle">OFF</span>
          </div>
        </div>
        <button class="btn-settings" onclick="window.location='/settings'">
          <svg viewBox="0 0 24 24"><path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.96 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.96 4.46,5.05 4.34,5.27L2.34,8.73C2.21,8.95 2.27,9.22 2.46,9.37L4.57,11C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.21,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z" /></svg>
          System Settings
        </button>
      </div>
    </div>
    <script>
      const updateBadge = (id, state) => {
        const el = document.getElementById(id);
        const isActive = state.toUpperCase() === "ON";
        el.innerText = state.toUpperCase();
        el.className = isActive ? "badge badge-active" : "badge badge-idle";
      };
      async function toggleLED(id) {
        try {
          const res = await fetch(`/toggle?led=${id}`);
          const data = await res.json();
          updateBadge("l1", data.led1);
          updateBadge("l2", data.led2);
        } catch (e) { console.error("Control error:", e); }
      }
      async function fetchMetrics() {
        try {
          const res = await fetch("/sensors");
          const data = await res.json();
          document.getElementById("temp").innerText = data.temp;
          document.getElementById("hum").innerText = data.hum;
        } catch (e) { console.error("Poll error:", e); }
      }
      window.onload = () => {
        document.getElementById("temp").innerText = ")rawliteral";
  
  ptr += String(temperature, 1);
  
  ptr += R"rawliteral(";
        document.getElementById("hum").innerText = ")rawliteral";
  
  ptr += String(humidity, 1);
  
  ptr += R"rawliteral(";
        updateBadge("l1", ")rawliteral";
  
  ptr += led1;
  
  ptr += R"rawliteral(");
        updateBadge("l2", ")rawliteral";
  
  ptr += led2;
  
  ptr += R"rawliteral(");
        fetchMetrics();
      };
      setInterval(fetchMetrics, 3000);
    </script>
  </body>
</html>
)rawliteral";

  return ptr;
}

String settingsPage()
{
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>System Settings | Wi-Fi</title>
  <style>
    :root {
      --nuxt-green: #00dc82;
      --nuxt-dark: #020420;
      --text-main: #e2e8f0;
      --text-dim: #94a3b8;
      --border-color: rgba(0, 220, 130, 0.2);
      --input-bg: rgba(255, 255, 255, 0.05);
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: "Inter", "Segoe UI", system-ui, sans-serif;
      background-color: var(--nuxt-dark);
      background-image: radial-gradient(circle at 50% 50%, #002e3b 0%, #020420 100%);
      color: var(--text-main);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      overflow-x: hidden;
    }

    .app-card {
      background: rgba(2, 4, 32, 0.8);
      border: 1px solid var(--border-color);
      border-radius: 16px;
      padding: 2.5rem;
      width: 100%;
      max-width: 380px;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
      backdrop-filter: blur(8px);
      text-align: center;
    }

    header h1 {
      margin-bottom: 0.5rem;
      font-size: 1.25rem;
      letter-spacing: 0.05em;
      color: var(--nuxt-green);
      font-weight: 600;
      text-transform: uppercase;
    }

    header p {
      margin-bottom: 2rem;
      font-size: 0.875rem;
      color: var(--text-dim);
    }

    /* Form Elements */
    input[type=text], input[type=password] {
      width: 100%;
      padding: 12px 16px;
      background: var(--input-bg);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 10px;
      color: #fff;
      font-size: 1rem;
      margin-bottom: 1rem;
      outline: none;
      transition: all 0.2s;
    }

    input[type=text]:focus, input[type=password]:focus {
      border-color: var(--nuxt-green);
      background: rgba(0, 220, 130, 0.05);
      box-shadow: 0 0 0 2px rgba(0, 220, 130, 0.2);
    }

    .btn-submit {
      width: 100%;
      background: var(--nuxt-green);
      color: var(--nuxt-dark);
      border: none;
      padding: 1rem;
      border-radius: 10px;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.2s;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      margin-bottom: 0.75rem;
    }

    .btn-submit:hover { filter: brightness(1.1); }
    .btn-submit:active { transform: scale(0.98); }

    .btn-back {
      background: transparent;
      color: var(--text-dim);
      border: 1px solid rgba(255, 255, 255, 0.1);
      padding: 0.75rem;
      width: 100%;
      border-radius: 10px;
      cursor: pointer;
      font-weight: 500;
      transition: all 0.2s;
    }

    .btn-back:hover {
      color: #fff;
      border-color: rgba(255, 255, 255, 0.3);
    }

    #msg {
      margin-top: 1.5rem;
      font-size: 0.875rem;
      color: var(--nuxt-green);
      min-height: 1.2rem;
    }

    @media (max-width: 480px) {
      .app-card { padding: 1.5rem; border: none; background: transparent; box-shadow: none; }
    }
  </style>
</head>
<body>
  <div class="app-card">
    <header>
      <h1>Network Config</h1>
      <p>Configure Wi-Fi Credentials</p>
    </header>

    <form id="wifiForm">
      <input name="ssid" id="ssid" type="text" placeholder="Wi-Fi Name (SSID)" required>
      <input name="password" id="pass" type="password" placeholder="Password">
      
      <button type="submit" class="btn-submit">Connect Device</button>
      <button type="button" class="btn-back" onclick="window.location='/'">Back to Dashboard</button>
    </form>

    <div id="msg"></div>
  </div>

  <script>
    document.getElementById('wifiForm').onsubmit = function(e){
      e.preventDefault();
      const msgEl = document.getElementById('msg');
      msgEl.innerText = "Applying settings...";
      
      let ssid = document.getElementById('ssid').value;
      let pass = document.getElementById('pass').value;
      
      fetch('/connect?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass))
        .then(r=>r.text())
        .then(msg=>{
          msgEl.innerText = msg;
        })
        .catch(err => {
          msgEl.style.color = "#ff4d4d";
          msgEl.innerText = "Error sending request";
        });
    };
  </script>
</body>
</html>
)rawliteral";
}

// ========== Handlers ==========
void handleRoot() { mainServer.send(200, "text/html", mainPage()); }

void handleToggle()
{
  int led = mainServer.arg("led").toInt();
  if (led == 1)
  {
    led1_state = !led1_state;
    digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
  }
  else if (led == 2)
  {
    led2_state = !led2_state;
    if (led2_state)
      neo.setPixelColor(0, neo.Color(0, 150, 0)); // green on
    else
      neo.setPixelColor(0, 0);
    neo.show();
  }
  mainServer.send(200, "application/json",
              "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}");
}

void handleSensors()
{
  SensorData_t sensorData;
  xQueuePeek(xSensorQueue, &sensorData, 0);
  float t = sensorData.temperature;
  float h = sensorData.humidity;
  String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
  mainServer.send(200, "application/json", json);
}

void handleSettings() { mainServer.send(200, "text/html", settingsPage()); }

void handleConnect()
{
  Serial.print("[CONNECT] argCount=");
  Serial.println(mainServer.args());
  for (int i = 0; i < mainServer.args(); i++)
  {
    Serial.print("[CONNECT] ");
    Serial.print(mainServer.argName(i));
    Serial.print("='");
    Serial.print(mainServer.arg(i));
    Serial.println("'");
  }

  wifi_ssid = mainServer.arg("ssid");
  wifi_password = mainServer.arg("pass");

  wifi_ssid.trim();
  wifi_password.trim();

  if (wifi_ssid.isEmpty())
  {
    Serial.println("[CONNECT] missing ssid parameter");
    mainServer.send(400, "text/plain", "SSID is required");
    return;
  }

  Serial.print("[CONNECT] SSID='\"");
  Serial.print(wifi_ssid);
  Serial.print("\"' PASS_LEN=");
  Serial.println(wifi_password.length());

  mainServer.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

// ========== WiFi ==========
void setupServer()
{
  mainServer.on("/", HTTP_GET, handleRoot);
  mainServer.on("/toggle", HTTP_GET, handleToggle);
  mainServer.on("/sensors", HTTP_GET, handleSensors);
  mainServer.on("/settings", HTTP_GET, handleSettings);
  mainServer.on("/connect", HTTP_GET, handleConnect);
  mainServer.begin();
}

void startAccessPoint()
{
  WiFi.mode(WIFI_AP);
  xSemaphoreTake(xMutexWifi, portMAX_DELAY);
  WiFi.softAP(ssid.c_str(), password.c_str());
  xSemaphoreGive(xMutexWifi);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi()
{ 
  WiFi.mode(WIFI_STA);
  if (wifi_password.isEmpty())
  {
    WiFi.begin(wifi_ssid.c_str());
  }
  else
  {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
  Serial.println();

  last_wifi_status_log_ms = 0;
}

// ========== Main task ==========
void main_server_task(void *pvParameters)
{
  pinMode(BOOT_PIN, INPUT_PULLUP);

  pinMode(LED1_PIN, INPUT);
  delay(10);
  led1_state = (digitalRead(LED1_PIN) == HIGH);

  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);

  neo.begin();
  if (led2_state)
    neo.setPixelColor(0, neo.Color(0, 150, 0));
  else
    neo.setPixelColor(0, 0);
  neo.show();

  startAccessPoint();
  setupServer();

  while (1)
  {
    mainServer.handleClient();

    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW)
      {
        if (!isAPMode)
        {
          startAccessPoint();
          setupServer();
        }
      }
    }

    if (connecting)
    {
      uint8_t wifiStatus = WiFi.status();
      if (millis() - last_wifi_status_log_ms >= 1000)
      {
        if (wifiStatus == WL_IDLE_STATUS) Serial.println(" (IDLE)");
        else if (wifiStatus == WL_NO_SSID_AVAIL) Serial.println(" (NO_SSID)");
        else if (wifiStatus == WL_SCAN_COMPLETED) Serial.println(" (SCAN_DONE)");
        else if (wifiStatus == WL_CONNECTED) Serial.println(" (CONNECTED)");
        else if (wifiStatus == WL_CONNECT_FAILED) Serial.println(" (CONNECT_FAILED)");
        else if (wifiStatus == WL_CONNECTION_LOST) Serial.println(" (CONNECTION_LOST)");
        else if (wifiStatus == WL_DISCONNECTED) Serial.println(" (DISCONNECTED)");
        else Serial.println();
        last_wifi_status_log_ms = millis();
      }
      
      if (wifiStatus == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true;

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
      }
      else if (millis() - connect_start_ms > 10000)
      { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAccessPoint();
        setupServer();
        connecting = false;
        isWifiConnected = false;
      }
    }

    vTaskDelay(20); // avoid watchdog reset
  }
}
