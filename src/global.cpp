#include "global.h"
#include "secrets.h"
#include "config.h"

bool led1_state = false;
bool led2_state = false;

String WIFI_SSID         = WIFI_STA_SSID;
String WIFI_PASS         = WIFI_STA_PASS;
String CORE_IOT_TOKEN    = COREIOT_TOKEN;
String CORE_IOT_SERVER   = COREIOT_SERVER;
String CORE_IOT_PORT     = String(COREIOT_PORT);

String ssid     = WIFI_AP_SSID;
String password = WIFI_AP_PASS;
String wifi_ssid;
String wifi_password;
boolean isWifiConnected = false;
QueueHandle_t xSensorQueue = xQueueCreate(1, sizeof(SensorData_t));
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();
SemaphoreHandle_t xMutexWifi = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphoreConfiguring = xSemaphoreCreateBinary();

// Sync primitives for LED and NeoPixel tasks
SemaphoreHandle_t xSemLED = xSemaphoreCreateBinary();
SemaphoreHandle_t xSemNeo = xSemaphoreCreateBinary();

// ML result queue
QueueHandle_t xMLQueue = xQueueCreate(1, sizeof(MLResult_t));

// Mutex for shared actuator state (led1_state, led2_state, autoMode)
SemaphoreHandle_t xMutexActuatorState = xSemaphoreCreateMutex();

// AUTO mode flag (always read/written under xMutexActuatorState)
bool autoMode = false;
