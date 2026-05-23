#ifndef __COREIOT_H__
#define __COREIOT_H__

#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>


/** Fallback MQTT path: publishes 7-field telemetry JSON to a local broker via raw PubSubClient every COREIOT_INTERVAL_MS. */
void coreiot_local_task(void *pvParameters);

#endif