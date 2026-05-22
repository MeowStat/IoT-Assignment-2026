#pragma once
#include "config.h"

static inline const char* getVineyardState(float temp, float humidity) {
    if (temp > TEMP_CRITICAL_MIN || humidity >= HUMID_CRITICAL_MIN) return "CRITICAL";
    if ((temp > TEMP_HOT_MIN && temp <= TEMP_WARNING_MAX) ||
        (humidity > HUMID_IDEAL_MAX && humidity <= HUMID_WARNING_MAX)) return "WARNING";
    return "NORMAL";
}
