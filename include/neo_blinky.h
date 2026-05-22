#ifndef __NEO_BLINKY__
#define __NEO_BLINKY__
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define NEO_PIN 45
#define LED_COUNT 1 

void neo_blinky(void *pvParameters);
void neoSetCycleSpeed(uint16_t speed_ms);
void neoSetBrightness(uint8_t level);

#endif
