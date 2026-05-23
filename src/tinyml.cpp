#include "tinyml.h"
#include "global.h"
#include "config.h"
#include "ml_result.h"

namespace {
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];

    // Set in setupTinyML() once tensor shape is known.
    int g_num_outputs = 1;
}

// Class labels for multi-class models (ordered to match training pipeline).
// Unused when the pre-trained model has only 1 output.
static const char* CLASS_LABELS[4] = {
    "Normal", "Disease Risk", "Heat Stress", "Cold Stress"
};

void setupTinyML() {
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report(
            "Model schema version mismatch (%d vs %d)",
            model->version(), TFLITE_SCHEMA_VERSION
        );
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input  = interpreter->input(0);
    output = interpreter->output(0);

    // Detect output topology so the task can branch later.
    g_num_outputs = (output->dims->size > 0)
        ? output->dims->data[output->dims->size - 1] 
        : 1;
    Serial.printf("[ML] output dims_size=%d, last_dim=%d → %s\n",
        output->dims->size, 
        g_num_outputs,
        g_num_outputs >= 2 ? "classifier" : "anomaly-score");
}

void tiny_ml_task(void *pvParameters) {
    setupTinyML();

    if (interpreter == nullptr || input == nullptr || output == nullptr) {
        Serial.println("[ML] Init failed — task suspended");
        vTaskSuspend(NULL);
    }

    while (1) {
        // Safe defaults guard against the first cycle where SensorTask
        // hasn't published yet (xQueuePeek returns pdFALSE).
        SensorData_t sensorData = {0.0f, 0.0f};
        if (xQueuePeek(xSensorQueue, &sensorData, pdMS_TO_TICKS(100)) != pdTRUE) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        input->data.f[0] = sensorData.temperature / ML_TEMP_NORM_DIVISOR;
        input->data.f[1] = sensorData.humidity    / ML_HUMID_NORM_DIVISOR;

        if (interpreter->Invoke() != kTfLiteOk) {
            error_reporter->Report("Invoke failed");
            vTaskDelay(pdMS_TO_TICKS(ML_INFERENCE_PERIOD_MS));
            continue;
        }

        MLResult_t result = {"Normal", 0.0f};

        if (g_num_outputs >= 2) {
            int best = 0;
            float best_score = output->data.f[0];
            int n = (g_num_outputs < 4) ? g_num_outputs : 4;
            for (int i = 1; i < n; i++) {
                if (output->data.f[i] > best_score) {
                    best_score = output->data.f[i];
                    best = i;
                }
            }
            strncpy(result.label, CLASS_LABELS[best], sizeof(result.label) - 1);
            result.confidence = best_score;
        } else {
            float score = output->data.f[0];
            const bool anomaly = score > ML_ANOMALY_THRESHOLD;
            strncpy(result.label, anomaly ? "Anomaly" : "Normal",
                    sizeof(result.label) - 1);
            result.confidence = score;
        }
        result.label[sizeof(result.label) - 1] = '\0';

        xQueueOverwrite(xMLQueue, &result);
        Serial.printf("[ML] %s (%.3f)\n", result.label, result.confidence);

        vTaskDelay(pdMS_TO_TICKS(ML_INFERENCE_PERIOD_MS));
    }
}
