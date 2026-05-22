#pragma once

typedef struct {
    char label[24];     // "Normal" | "Disease Risk" | "Heat Stress" | "Cold Stress" | "Anomaly"
    float confidence;   // 0.0 – 1.0 (argmax score for multi-class; anomaly score for single-output)
} MLResult_t;
