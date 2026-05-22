#include "temp_humi_monitor.h"
#include "config.h"

DHT20 dht;
LiquidCrystal_I2C lcd(33, 16, 2);

void temp_humi_monitor(void *pvParameters) {
  Serial.println("[SensorTask] Starting...");

  Wire.begin(11, 12);
  dht.begin();

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  Serial.println("[SensorTask] LCD + DHT20 ready");

  while (1) {
    int status = dht.read();
    float temperature = dht.getTemperature();
    float humidity    = dht.getHumidity();

    if (status != DHT20_OK) {
      Serial.println("[SensorTask] DHT20 read failed");
      lcd.setCursor(0, 0);
      lcd.print("Error: DHT20    ");
      lcd.setCursor(0, 1);
      lcd.print("Read failed     ");
    } else {
      SensorData_t data = { temperature, humidity };
      xQueueOverwrite(xSensorQueue, &data);

      xSemaphoreGive(xSemLED);
      xSemaphoreGive(xSemNeo);

      Serial.printf("[SensorTask] T: %.1f C  H: %.1f%%\n", temperature, humidity);

      const char* state;
      if (temperature > TEMP_CRITICAL_MIN || humidity >= HUMID_CRITICAL_MIN) {
        state = "CRITICAL";
      } else if ((temperature > TEMP_HOT_MIN && temperature <= TEMP_WARNING_MAX) ||
                 (humidity > HUMID_IDEAL_MAX && humidity <= HUMID_WARNING_MAX)) {
        state = "WARNING";
      } else {
        state = "NORMAL";
      }

      lcd.setCursor(0, 0);
      lcd.printf("T:%.1fC  H:%.1f%%", temperature, humidity);
      lcd.setCursor(0, 1);
      lcd.printf("State:%-8s", state);
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL_MS));
  }
}
