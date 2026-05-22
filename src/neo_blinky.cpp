#include "neo_blinky.h"
#include "global.h"
#include "config.h"

Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

void neo_blinky(void *pvParameters) {
  strip.begin();
  strip.clear();
  strip.show();

  while (1) {
    xSemaphoreTake(xSemNeo, portMAX_DELAY);

    SensorData_t data;
    if (xQueuePeek(xSensorQueue, &data, 0) != pdTRUE) continue;

    float rh = data.humidity;
    uint32_t color;

    if (rh < HUMID_DRY_MAX) {
      color = strip.Color(255, 0, 0);   // Red — dry, needs irrigation
    } else if (rh <= HUMID_IDEAL_MAX) {
      color = strip.Color(0, 255, 0);   // Green — ideal range
    } else {
      color = strip.Color(0, 0, 255);   // Blue — high humidity, fungal risk
    }

    strip.setPixelColor(0, color);
    strip.show();
  }
}
