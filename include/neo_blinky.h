#ifndef __NEO_BLINKY__
#define __NEO_BLINKY__
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define NEO_PIN 45
#define LED_COUNT 1

// Sets NeoPixel color based on current humidity: red < 40%, green 40–75%, blue > 75%
void neo_blinky(void *pvParameters);

#endif
