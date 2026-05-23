
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <task_handler.h>

extern AsyncWebServer webServer;
extern AsyncWebSocket ws;

/** Closes all WebSocket clients and stops the HTTP server. */
void Webserver_stop();
/** Starts the HTTP+WS server if not already running; calls ElegantOTA.loop(). */
void Webserver_reconnect();
/** Sends a raw string to all connected WebSocket clients. */
void Webserver_sendata(String data);
/** Broadcasts a full sensor+state+actuator JSON payload to all WebSocket clients. */
void pushSensorData();
/** FreeRTOS task: waits for WiFi, starts AsyncWebServer on port 80, pushes sensor data every 5s. */
void webserver_task(void* param);

#endif
