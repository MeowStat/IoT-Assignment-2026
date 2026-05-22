#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LED_GPIO 48

// Morse code blinking function
// pvParameters: pointer to const char* message (e.g., "HELLO")
void led_blinky(void *pvParameters);

// Helper functions
const char* getMorseCode(char c);
void blinkMorseChar(const char* morse);

#endif