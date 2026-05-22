#include "led_blinky.h"
#include "global.h"
#include "config.h"

void led_blinky(void *pvParameters) {
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);

  while (1) {
    xSemaphoreTake(xSemLED, portMAX_DELAY);

    SensorData_t data;
    if (xQueuePeek(xSensorQueue, &data, 0) != pdTRUE) continue;

    float temp = data.temperature;

    if (temp >= TEMP_COLD_MAX && temp <= TEMP_HOT_MIN) {
      // Normal range — steady on for one sensor cycle
      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL_MS));
    } else {
      // Slow blink < 20°C, fast blink > 35°C
      uint32_t period = (temp < TEMP_COLD_MAX) ? LED_SLOW_PERIOD_MS : LED_FAST_PERIOD_MS;
      uint32_t elapsed = 0;
      while (elapsed < SENSOR_INTERVAL_MS) {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(period / 2));
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(period / 2));
        elapsed += period;
      }
    }
  }
}
