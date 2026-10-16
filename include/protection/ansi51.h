#ifndef ANSI51_H
#define ANSI51_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef enum {
    CURVE_IEC_STANDARD_INVERSE = 0,
    CURVE_IEC_VERY_INVERSE = 1,
    CURVE_IEC_EXTREMELY_INVERSE = 2,
    CURVE_IEEE_MODERATELY_INVERSE = 3,
    CURVE_IEEE_VERY_INVERSE = 4,
    CURVE_IEEE_EXTREMELY_INVERSE = 5
} idmt_curve_type_t;

typedef struct {
    ansi51_config_t config;
    bool pickup_state;
    uint32_t pickup_time_ms;
    float operate_time_ms;
    float current_multiple;
    uint8_t phase_fault;
} ansi51_state_t;

void ansi51_init(ansi51_state_t* state, const ansi51_config_t* config);
bool ansi51_evaluate(ansi51_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms);
void ansi51_reset(ansi51_state_t* state);
float ansi51_calculate_trip_time(const ansi51_state_t* state, float current_multiple);
uint8_t ansi51_get_faulted_phase(const ansi51_state_t* state);

#endif
