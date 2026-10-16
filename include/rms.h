#ifndef RMS_H
#define RMS_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    float buffer[RMS_WINDOW_SAMPLES];
    uint16_t index;
    uint16_t count;
    float sum_squares;
    float rms_value;
    bool ready;
} rms_calculator_t;

void rms_init(rms_calculator_t* rms);
void rms_reset(rms_calculator_t* rms);
bool rms_update(rms_calculator_t* rms, float sample);
float rms_get_value(const rms_calculator_t* rms);
bool rms_is_ready(const rms_calculator_t* rms);

#endif
