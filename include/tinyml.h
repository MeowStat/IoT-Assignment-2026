#ifndef __TINY_ML__
#define __TINY_ML__

#include <Arduino.h>

#include "dht_anomaly_model.h"
#include "global.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

/** Initializes the TensorFlow Lite Micro interpreter with the embedded model; logs output topology once at boot. */
void setupTinyML();
/** Runs inference on current sensor data every ML_INFERENCE_PERIOD_MS; writes MLResult_t {label, confidence} to xMLQueue. */
void tiny_ml_task(void *pvParameters);

#endif