#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "ml_result.h"

#define LED1_PIN 7     // pump - controlled by AUTO + telemetry
#define LED2_PIN 8     // fan - controlled by AUTO + telemetry

typedef struct {
    float temperature;
    float humidity;
} SensorData_t;

extern QueueHandle_t xSensorQueue;

extern bool led1_state;
extern bool led2_state;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern SemaphoreHandle_t xMutexWifi;
extern SemaphoreHandle_t xSemaphoreConfiguring;

// Sync primitives for LED and NeoPixel tasks
extern SemaphoreHandle_t xSemLED;
extern SemaphoreHandle_t xSemNeo;

// ML result queue 
extern QueueHandle_t xMLQueue;

// Mutex for shared actuator state (led1_state, led2_state, autoMode)
extern SemaphoreHandle_t xMutexActuatorState;

// AUTO mode flag (always read/written under xMutexActuatorState)
extern bool autoMode;
#endif
