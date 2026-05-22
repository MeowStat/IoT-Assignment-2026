#pragma once

// Sensor
#define SENSOR_INTERVAL_MS      2000

// Temperature thresholds (FR1)
#define TEMP_COLD_MAX           20.0f
#define TEMP_HOT_MIN            35.0f
#define LED_SLOW_PERIOD_MS      2000
#define LED_FAST_PERIOD_MS      200

// Humidity thresholds (FR2)
#define HUMID_DRY_MAX           40.0f
#define HUMID_IDEAL_MAX         75.0f

// LCD state thresholds (FR3)
#define TEMP_WARNING_MAX        37.0f
#define TEMP_CRITICAL_MIN       38.0f
#define HUMID_WARNING_MAX       85.0f
#define HUMID_CRITICAL_MIN      85.0f

// Auto mode hysteresis (FR4.4) — separate ON/OFF thresholds prevent flapping
#define AUTO_PUMP_HUMID_ON      40.0f   // pump ON  when RH < 40
#define AUTO_PUMP_HUMID_OFF     45.0f   // pump OFF when RH >= 45
#define AUTO_FAN_HUMID_ON       80.0f   // fan  ON  when RH > 80
#define AUTO_FAN_HUMID_OFF      75.0f   // fan  OFF when RH <= 75

// ML normalization + threshold (FR5)
#define ML_TEMP_NORM_DIVISOR    45.0f
#define ML_HUMID_NORM_DIVISOR   100.0f
#define ML_ANOMALY_THRESHOLD    0.5f    // single-output models only
#define ML_INFERENCE_PERIOD_MS  5000

// CoreIOT publish interval (FR6)
#define COREIOT_INTERVAL_MS     7000

// WiFi AP (FR4.1) — also set via platformio.ini build_flags for task_wifi.cpp
#define WIFI_AP_SSID            "FarmGenius_AP_01"
#define WIFI_AP_PASS            "farmgenius2026"
