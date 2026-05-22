#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LED_GPIO 48

// Blinks at rate based on current temperature: slow < 20°C, steady 20–35°C, fast > 35°C
void led_blinky(void *pvParameters);

#endif
