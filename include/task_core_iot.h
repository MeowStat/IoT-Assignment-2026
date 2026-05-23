#ifndef __TASK_CORE_IOT_H__
#define __TASK_CORE_IOT_H__

#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <HTTPClient.h>
#include "task_check_info.h"

/** Publishes telemetry + ML result + actuator state to ThingsBoard every COREIOT_INTERVAL_MS. Waits for WiFi STA via semaphore gate. */
void coreiot_task(void *pvParameters);
/** Sends a single field to ThingsBoard (mode = "telemetry" or "attribute"). */
void CORE_IOT_sendata(String mode, String feed, String data);
/** Reconnects to ThingsBoard broker if disconnected; calls tb.loop() when connected. */
void CORE_IOT_reconnect();

#endif
