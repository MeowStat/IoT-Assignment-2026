#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"

void setup()
{
  Serial.begin(115200);

  // Priority 3 — critical timing tasks
  xTaskCreate(temp_humi_monitor, "SensorTask", 4096, NULL, 3, NULL);
  xTaskCreate(wifi_task, "WiFiTask", 4096, NULL, 3, NULL);

  // Priority 2 — normal operation tasks
  xTaskCreate(led_blinky, "LEDTask", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "NeoTask", 2048, NULL, 2, NULL);
  xTaskCreate(webserver_task, "WebTask", 8192, NULL, 2, NULL);
  xTaskCreate(tiny_ml_task, "MLTask", 4096, NULL, 2, NULL);
  // Primary MQTT path: ThingsBoard cloud (app.coreiot.io style)
  // xTaskCreate(coreiot_task, "CoreIOTCloud", 4096, NULL, 2, NULL);
  
  // To activate: comment the line above and uncomment the line below.
  xTaskCreate(coreiot_local_task, "CoreIOTLocal", 4096, NULL, 2, NULL); // Fallback MQTT path: raw PubSubClient to any local broker.

  // Priority 1 — maintenance tasks
  xTaskCreate(Task_Toogle_BOOT, "BootTask", 4096, NULL, 1, NULL);
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}
