#include "coreiot.h"
#include "mainserver.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "192.168.1.31";
const char* coreIOT_Token = "AnGdNEIQNZjoIejnBilZ";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    //if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String clientId = "ESP32-" + mac;  // e.g. "ESP32-A4CF125B3C2D"


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


void callback(char* topic, byte* payload, unsigned int length) {
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
    led1_state = newState;
    digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);

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


void setup_coreiot(){
  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters) {

    setup_coreiot();

    const TickType_t telemetryInterval = pdMS_TO_TICKS(5000);
    TickType_t lastTelemetry = xTaskGetTickCount() - telemetryInterval;

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        if ((xTaskGetTickCount() - lastTelemetry) >= telemetryInterval) {
            lastTelemetry = xTaskGetTickCount();

            SensorData_t sensorData;
            xQueuePeek(xSensorQueue, &sensorData, 0);
            float t = sensorData.temperature;
            float h = sensorData.humidity;
            String payload = "{\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
            String mac = WiFi.macAddress();
            mac.replace(":", "");
            String telemetryTopic = "devices/" + mac + "/telemetry";
            client.publish(telemetryTopic.c_str(), payload.c_str());
            Serial.println("Published payload: " + payload);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
