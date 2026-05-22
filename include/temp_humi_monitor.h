#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
// #include "DHT.h"
#include "global.h"
#include "DHT20.h"

// #define DHT_PIN GPIO_NUM_3

void temp_humi_monitor(void *pvParameters);


#endif
