#include <task_handler.h>
#include "global.h"
#include "config.h"
#include "task_webserver.h"

void handleWebSocketMessage(String message) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, message)) {
        Serial.println("[WS] JSON parse error");
        return;
    }

    const String page = doc["page"].as<String>();

    // ---- spec-mandated pump/fan ----
    if (page == "actuator") {
        const String name = doc["value"]["name"].as<String>();
        if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (name == "pump") {
                led1_state = !led1_state;
                pinMode(LED1_PIN, OUTPUT);
                digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
            } else if (name == "fan") {
                led2_state = !led2_state;
                pinMode(LED2_PIN, OUTPUT);
                digitalWrite(LED2_PIN, led2_state ? HIGH : LOW);
            }
            xSemaphoreGive(xMutexActuatorState);
        }
        pushSensorData();
    }
    // ---- dynamic user-added relays (lab feature, kept) ----
    else if (page == "device") {
        JsonObject value = doc["value"];
        if (!value.containsKey("gpio") || !value.containsKey("status")) {
            Serial.println("[WS] device payload missing gpio/status");
            return;
        }
        int gpio = value["gpio"];
        String status = value["status"].as<String>();
        Serial.printf("[WS] device GPIO %d → %s\n", gpio, status.c_str());
        pinMode(gpio, OUTPUT);
        if (status.equalsIgnoreCase("ON"))       digitalWrite(gpio, HIGH);
        else if (status.equalsIgnoreCase("OFF")) digitalWrite(gpio, LOW);
    }
    // ---- AUTO mode toggle ----
    else if (page == "auto") {
        if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
            autoMode = !autoMode;
            xSemaphoreGive(xMutexActuatorState);
        }
        pushSensorData();
    }
    // ---- settings  ----
    else if (page == "setting") {
        WIFI_SSID       = doc["value"]["ssid"].as<String>();
        WIFI_PASS       = doc["value"]["password"].as<String>();
        CORE_IOT_TOKEN  = doc["value"]["token"].as<String>();
        CORE_IOT_SERVER = doc["value"]["server"].as<String>();
        CORE_IOT_PORT   = doc["value"]["port"].as<String>();

        Serial.println("[WS] settings received");
        xSemaphoreTake(xSemaphoreConfiguring, portMAX_DELAY);
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);
        xSemaphoreGive(xSemaphoreConfiguring);

        ws.textAll("{\"page\":\"setting_saved\",\"status\":\"ok\"}");
    }
    else {
        Serial.printf("[WS] unknown page: %s\n", page.c_str());
    }
}
