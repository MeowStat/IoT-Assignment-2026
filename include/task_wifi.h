#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#include <WiFi.h>
#include <task_check_info.h>
#include <task_webserver.h>

/** Attempts to reconnect to STA WiFi; returns true if connected. */
extern bool Wifi_reconnect();
/** Starts the ESP32 in Access Point mode with the configured SSID/password. */
extern void startAP();
/** Manages WiFi connection (STA → AP fallback); gives xBinarySemaphoreInternet when STA is up. */
void wifi_task(void* param);

#endif