#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdint.h>
#include <stdbool.h>
#include "ansi50.h"
#include "ansi51.h"
#include "config.h"

typedef enum {
    TRIP_REASON_NONE = 0,
    TRIP_REASON_ANSI50 = 1,
    TRIP_REASON_ANSI51 = 2,
    TRIP_REASON_MANUAL = 3
} trip_reason_t;

typedef struct {
    trip_reason_t reason;
    uint8_t faulted_phase;
    float fault_current;
    uint32_t timestamp_ms;
    uint32_t trip_time_ms;
} trip_event_t;

typedef struct {
    ansi50_state_t ansi50;
    ansi51_state_t ansi51;
    trip_event_t last_trip;
    uint32_t trip_count;
} protection_state_t;

void protection_init(protection_state_t* state);
bool protection_evaluate(protection_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms, trip_reason_t* reason);
void protection_reset(protection_state_t* state);
void protection_get_last_trip(const protection_state_t* state, trip_event_t* event);

#endif
