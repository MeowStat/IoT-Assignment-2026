#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"

// Morse code message to blink
const char* MORSE_MESSAGE = "SOS";

void setup()
{
  Serial.begin(115200);
  // check_info_File(0);

  // xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)MORSE_MESSAGE, 1, NULL);
  // xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 1, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 1, NULL);
  xTaskCreate(wifi_task, "Task WiFi", 4096, NULL, 3, NULL);
  xTaskCreate(webserver_task, "Task Webserver", 8192, NULL, 2, NULL);
  xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_local_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}
