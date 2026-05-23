#include "task_webserver.h"
#include "global.h"
#include "ml_result.h"
#include "config.h"
#include "vineyard_state.h"

AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); // Gửi đến tất cả client đang kết nối
        Serial.println("📤 Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        Serial.println("⚠️ Không có client WebSocket nào đang kết nối!");
    }
}

void pushSensorData()
{
    if (!webserver_isrunning) return;

    SensorData_t s = {0.0f, 0.0f};
    MLResult_t   m = {"Unknown", 0.0f};
    xQueuePeek(xSensorQueue, &s, 0);
    xQueuePeek(xMLQueue,     &m, 0);

    bool pump = false, fan = false, autoM = false;
    if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
        pump  = led1_state;
        fan   = led2_state;
        autoM = autoMode;
        xSemaphoreGive(xMutexActuatorState);
    }
    const char* state = getVineyardState(s.temperature, s.humidity);

    char buf[256];
    snprintf(buf, sizeof(buf),
        "{\"page\":\"sensor\",\"temp\":%.1f,\"humi\":%.1f,"
        "\"state\":\"%s\",\"anomaly\":\"%s\",\"score\":%.2f,"
        "\"pump\":%s,\"fan\":%s,\"auto\":%s}",
        s.temperature, s.humidity, state, m.label, m.confidence,
        pump  ? "true" : "false",
        fan   ? "true" : "false",
        autoM ? "true" : "false");
    Webserver_sendata(String(buf));
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        pushSensorData();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            // parseJson(message, true);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV()
{
    ws.onEvent(onEvent);
    webServer.addHandler(&ws);
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    webServer.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    webServer.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    webServer.begin();
    ElegantOTA.begin(&webServer);
    webserver_isrunning = true;
}

void Webserver_stop()
{
    ws.closeAll();
    webServer.end();
    webserver_isrunning = false;
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
}

void webserver_task(void* param)
{
    // Block until WiFi is connected — webServer.begin() calls into lwIP,
    // which crashes with "Invalid mbox" if the TCP/IP stack isn't up yet.
    xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY);
    xSemaphoreGive(xBinarySemaphoreInternet);

    xSemaphoreGive(xSemaphoreConfiguring);

    TickType_t xLastPush = xTaskGetTickCount();
    const TickType_t xPushInterval = pdMS_TO_TICKS(5000);

    while (true)
    {
        Webserver_reconnect();
        ws.cleanupClients(4);

        if (ws.count() > 0)
        {
            TickType_t now = xTaskGetTickCount();
            if ((now - xLastPush) >= xPushInterval)
            {
                xLastPush = now;
                pushSensorData();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
