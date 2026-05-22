#include "neo_blinky.h"

Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
volatile uint16_t cycleSpeed = 100;  // milliseconds per color step
volatile uint8_t brightness = 50;   // 0-255 brightness level (128 = 50% intensity)

void neo_blinky(void *pvParameters) {
  Serial.println("[NeoPixel Task] Started - Rainbow Cycle");
  
  strip.begin();
  strip.clear();
  strip.show();
  
  Serial.println("[NeoPixel Task] Strip initialized on GPIO 45");
  
  while (1) {
    // HSV: H(0-65536) = hue, S(255) = full saturation, V(255) = full brightness
    for (uint16_t i = 0; i < 65536; i += 256) {
      uint32_t color = strip.ColorHSV(i, 255, brightness);
      strip.setPixelColor(0, color);
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(cycleSpeed));
    }
  }
}

// Allow other tasks to adjust rainbow cycle speed
void neoSetCycleSpeed(uint16_t speed_ms) {
  cycleSpeed = speed_ms;
  Serial.print("[NeoPixel] Cycle speed changed to: ");
  Serial.print(speed_ms);
  Serial.println(" ms");
}

// Allow other tasks to adjust LED brightness (0-255)
void neoSetBrightness(uint8_t level) {
  brightness = level;
  Serial.print("[NeoPixel] Brightness changed to: ");
  Serial.print(level);
  Serial.println(" (0-255)");
}
