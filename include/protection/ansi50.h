#ifndef ANSI50_H
#define ANSI50_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    ansi50_config_t config;
    bool pickup_state;
    uint32_t pickup_time_ms;
    uint8_t phase_fault;
} ansi50_state_t;

void ansi50_init(ansi50_state_t* state, const ansi50_config_t* config);
bool ansi50_evaluate(ansi50_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms);
void ansi50_reset(ansi50_state_t* state);
uint8_t ansi50_get_faulted_phase(const ansi50_state_t* state);

#endif
