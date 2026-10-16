#include "protection/ansi50.h"
#include <string.h>
#include <math.h>

void ansi50_init(ansi50_state_t* state, const ansi50_config_t* config) {
    memcpy(&state->config, config, sizeof(ansi50_config_t));
    state->pickup_state = false;
    state->pickup_time_ms = 0;
    state->phase_fault = 0;
}

bool ansi50_evaluate(ansi50_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms) {
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
    
    if (max_current >= state->config.pickup_current) {
        if (!state->pickup_state) {
            state->pickup_state = true;
            state->pickup_time_ms = timestamp_ms;
            state->phase_fault = faulted_phase;
        }
        
        uint32_t elapsed_ms = timestamp_ms - state->pickup_time_ms;
        if (elapsed_ms >= state->config.delay_ms) {
            return true;
        }
    } else {
        state->pickup_state = false;
        state->pickup_time_ms = 0;
        state->phase_fault = 0;
    }
    
    return false;
}

void ansi50_reset(ansi50_state_t* state) {
    state->pickup_state = false;
    state->pickup_time_ms = 0;
    state->phase_fault = 0;
}

uint8_t ansi50_get_faulted_phase(const ansi50_state_t* state) {
    return state->phase_fault;
}
