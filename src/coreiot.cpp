#include "coreiot.h"
#include "ml_result.h"
#include "config.h"
#include "vineyard_state.h"

WiFiClient espClient;
PubSubClient client(espClient);


void coreiot_local_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    //if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String clientId = "ESP32-" + mac;


    if (client.connect(clientId.c_str())) {
        
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void coreiot_local_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract request ID for RPC response: topic = v1/devices/me/rpc/request/<id>
  String topicStr = String(topic);
  String requestId = topicStr.substring(topicStr.lastIndexOf('/') + 1);

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    // Example: {"method": "setStateLED", "params": "ON"}
    const char* params = doc["params"];

    bool newState = (strcmp(params, "ON") == 0);
    if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
        led1_state = newState;
        xSemaphoreGive(xMutexActuatorState);
    }
    digitalWrite(LED1_PIN, newState ? HIGH : LOW);

    Serial.print("Device LED ");
    Serial.println(newState ? "turned ON." : "turned OFF.");

    // Send RPC response back to CoreIOT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String responsePayload = String("{\"result\":") + (newState ? "ON" : "OFF") + "}";
    client.publish(responseTopic.c_str(), responsePayload.c_str());

  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}


void setup_coreiot_local(){
  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
  client.setCallback(coreiot_local_callback);

}

void coreiot_local_task(void *pvParameters) {
    setup_coreiot_local();

    const TickType_t interval = pdMS_TO_TICKS(COREIOT_INTERVAL_MS);
    TickType_t last = xTaskGetTickCount() - interval;

    while (1) {
        if (!client.connected()) coreiot_local_reconnect();
        client.loop();

        if ((xTaskGetTickCount() - last) >= interval) {
            last = xTaskGetTickCount();

            SensorData_t s = {0.0f, 0.0f};
            MLResult_t   m = {"Unknown", 0.0f};
            xQueuePeek(xSensorQueue, &s, 0);
            xQueuePeek(xMLQueue,     &m, 0);

            bool pump = false, fan = false;
            if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
                pump = led1_state; fan = led2_state;
                xSemaphoreGive(xMutexActuatorState);
            }
            const char* state = getVineyardState(s.temperature, s.humidity);

            char payload[256];
            snprintf(payload, sizeof(payload),
                "{\"temperature\":%.2f,\"humidity\":%.2f,"
                "\"vineyard_state\":\"%s\","
                "\"pump_status\":\"%s\",\"fan_status\":\"%s\","
                "\"anomaly_label\":\"%s\",\"anomaly_score\":%.3f}",
                s.temperature, s.humidity, state,
                pump ? "ON" : "OFF", fan ? "ON" : "OFF",
                m.label, m.confidence);

            String mac = WiFi.macAddress(); mac.replace(":", "");
            String topic = "devices/" + mac + "/telemetry";
            client.publish(topic.c_str(), payload);
            Serial.printf("[CoreIOT-Local] %s\n", payload);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
