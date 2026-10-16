#include "rms.h"
#include <math.h>
#include <string.h>

void rms_init(rms_calculator_t* rms) {
    memset(rms->buffer, 0, sizeof(rms->buffer));
    rms->index = 0;
    rms->count = 0;
    rms->sum_squares = 0.0f;
    rms->rms_value = 0.0f;
    rms->ready = false;
}

void rms_reset(rms_calculator_t* rms) {
    rms_init(rms);
}

bool rms_update(rms_calculator_t* rms, float sample) {
    float old_sample = rms->buffer[rms->index];
    
    rms->sum_squares -= old_sample * old_sample;
    rms->sum_squares += sample * sample;
    
    rms->buffer[rms->index] = sample;
    rms->index = (rms->index + 1) % RMS_WINDOW_SAMPLES;
    
    if (rms->count < RMS_WINDOW_SAMPLES) {
        rms->count++;
    }
    
    if (rms->count >= RMS_WINDOW_SAMPLES) {
        rms->rms_value = sqrtf(rms->sum_squares / RMS_WINDOW_SAMPLES);
        rms->ready = true;
        return true;
    }
    
    return false;
}

float rms_get_value(const rms_calculator_t* rms) {
    return rms->rms_value;
}

bool rms_is_ready(const rms_calculator_t* rms) {
    return rms->ready;
}
