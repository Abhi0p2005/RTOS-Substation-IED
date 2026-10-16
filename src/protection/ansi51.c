#include "protection/ansi51.h"
#include <string.h>
#include <math.h>

static float calculate_idmt_time(idmt_curve_type_t curve, float current_multiple, float tms) {
    float k, alpha, beta;
    
    switch (curve) {
        case CURVE_IEC_STANDARD_INVERSE:
            k = 0.14f;
            alpha = 0.02f;
            beta = 2.97f;
            break;
        case CURVE_IEC_VERY_INVERSE:
            k = 13.5f;
            alpha = 1.0f;
            beta = 1.0f;
            break;
        case CURVE_IEC_EXTREMELY_INVERSE:
            k = 80.0f;
            alpha = 2.0f;
            beta = 1.0f;
            break;
        case CURVE_IEEE_MODERATELY_INVERSE:
            k = 0.0515f;
            alpha = 0.02f;
            beta = 2.114f;
            break;
        case CURVE_IEEE_VERY_INVERSE:
            k = 19.61f;
            alpha = 2.0f;
            beta = 0.491f;
            break;
        case CURVE_IEEE_EXTREMELY_INVERSE:
            k = 28.2f;
            alpha = 2.0f;
            beta = 0.1217f;
            break;
        default:
            k = 0.14f;
            alpha = 0.02f;
            beta = 2.97f;
            break;
    }
    
    if (current_multiple <= 1.0f) {
        return INFINITY;
    }
    
    float time_sec = (k * tms) / (powf(current_multiple, alpha) - beta);
    return time_sec * 1000.0f;
}

void ansi51_init(ansi51_state_t* state, const ansi51_config_t* config) {
    memcpy(&state->config, config, sizeof(ansi51_config_t));
    state->pickup_state = false;
    state->pickup_time_ms = 0;
    state->operate_time_ms = 0.0f;
    state->current_multiple = 0.0f;
    state->phase_fault = 0;
}

bool ansi51_evaluate(ansi51_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms) {
    if (!state->config.enabled) {
        return false;
    }
    
    float max_current = ia_rms;
    uint8_t faulted_phase = 1;
    
    if (ib_rms > max_current) {
        max_current = ib_rms;
        faulted_phase = 2;
    }
    if (ic_rms > max_current) {
        max_current = ic_rms;
        faulted_phase = 3;
    }
    
    state->current_multiple = max_current / state->config.pickup_current;
    
    if (state->current_multiple >= 1.0f) {
        if (!state->pickup_state) {
            state->pickup_state = true;
            state->pickup_time_ms = timestamp_ms;
            state->phase_fault = faulted_phase;
            state->operate_time_ms = calculate_idmt_time(
                state->config.curve_type,
                state->current_multiple,
                state->config.time_multiplier
            );
        }
        
        uint32_t elapsed_ms = timestamp_ms - state->pickup_time_ms;
        if (elapsed_ms >= (uint32_t)state->operate_time_ms) {
            return true;
        }
    } else {
        state->pickup_state = false;
        state->pickup_time_ms = 0;
        state->phase_fault = 0;
        state->operate_time_ms = 0.0f;
    }
    
    return false;
}

void ansi51_reset(ansi51_state_t* state) {
    state->pickup_state = false;
    state->pickup_time_ms = 0;
    state->operate_time_ms = 0.0f;
    state->current_multiple = 0.0f;
    state->phase_fault = 0;
}

float ansi51_calculate_trip_time(const ansi51_state_t* state, float current_multiple) {
    return calculate_idmt_time(
        state->config.curve_type,
        current_multiple,
        state->config.time_multiplier
    );
}

uint8_t ansi51_get_faulted_phase(const ansi51_state_t* state) {
    return state->phase_fault;
}
