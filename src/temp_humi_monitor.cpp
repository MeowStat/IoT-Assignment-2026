#include "temp_humi_monitor.h"
#include "config.h"
#include "vineyard_state.h"

DHT20 dht;
LiquidCrystal_I2C lcd(33, 16, 2);

void temp_humi_monitor(void *pvParameters) {
  Serial.println("[SensorTask] Starting...");

  Wire.begin(11, 12);
  dht.begin();

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

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

      // AUTO mode hysteresis (FR4.4) — symmetric on/off so devices can turn back off.
      if (xSemaphoreTake(xMutexActuatorState, pdMS_TO_TICKS(100)) == pdTRUE) {
          if (autoMode) {
              if (humidity <  AUTO_PUMP_HUMID_ON  && !led1_state) {
                  led1_state = true;  digitalWrite(LED1_PIN, HIGH);
              } else if (humidity >= AUTO_PUMP_HUMID_OFF && led1_state) {
                  led1_state = false; digitalWrite(LED1_PIN, LOW);
              }
              if (humidity >  AUTO_FAN_HUMID_ON   && !led2_state) {
                  led2_state = true;  digitalWrite(LED2_PIN, HIGH);
              } else if (humidity <= AUTO_FAN_HUMID_OFF  && led2_state) {
                  led2_state = false; digitalWrite(LED2_PIN, LOW);
              }
          }
          xSemaphoreGive(xMutexActuatorState);
      }

      Serial.printf("[SensorTask] T: %.1f C  H: %.1f%%\n", temperature, humidity);

      const char* state = getVineyardState(temperature, humidity);

      lcd.setCursor(0, 0);
      lcd.printf("T:%.1fC  H:%.1f%%", temperature, humidity);
      lcd.setCursor(0, 1);
      lcd.printf("State:%-8s", state);
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL_MS));
  }
}
