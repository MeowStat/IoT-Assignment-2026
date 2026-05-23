
#ifndef __TASK_HANDLER_H__
#define __TASK_HANDLER_H__

#include <ArduinoJson.h>
#include <task_check_info.h>

/** Routes incoming WebSocket JSON to: actuator (pump/fan toggle), device (dynamic relay), auto (AUTO mode), setting (credentials save). */
extern void handleWebSocketMessage(String message);
#endif