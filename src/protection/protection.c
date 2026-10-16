#include "protection/protection.h"
#include <string.h>

void protection_init(protection_state_t* state) {
    ansi50_init(&state->ansi50, &g_protection_config.ansi50);
    ansi51_init(&state->ansi51, &g_protection_config.ansi51);
    memset(&state->last_trip, 0, sizeof(trip_event_t));
    state->trip_count = 0;
}

bool protection_evaluate(protection_state_t* state, float ia_rms, float ib_rms, float ic_rms, uint32_t timestamp_ms, trip_reason_t* reason) {
    bool trip = false;
    *reason = TRIP_REASON_NONE;
    
    if (ansi50_evaluate(&state->ansi50, ia_rms, ib_rms, ic_rms, timestamp_ms)) {
        trip = true;
        *reason = TRIP_REASON_ANSI50;
        state->last_trip.reason = TRIP_REASON_ANSI50;
        state->last_trip.faulted_phase = ansi50_get_faulted_phase(&state->ansi50);
        state->last_trip.fault_current = (ia_rms > ib_rms) ? 
            ((ia_rms > ic_rms) ? ia_rms : ic_rms) : 
            ((ib_rms > ic_rms) ? ib_rms : ic_rms);
        state->last_trip.timestamp_ms = timestamp_ms;
        state->last_trip.trip_time_ms = timestamp_ms - state->ansi50.pickup_time_ms;
        state->trip_count++;
    }
    else if (ansi51_evaluate(&state->ansi51, ia_rms, ib_rms, ic_rms, timestamp_ms)) {
        trip = true;
        *reason = TRIP_REASON_ANSI51;
        state->last_trip.reason = TRIP_REASON_ANSI51;
        state->last_trip.faulted_phase = ansi51_get_faulted_phase(&state->ansi51);
        state->last_trip.fault_current = (ia_rms > ib_rms) ? 
            ((ia_rms > ic_rms) ? ia_rms : ic_rms) : 
            ((ib_rms > ic_rms) ? ib_rms : ic_rms);
        state->last_trip.timestamp_ms = timestamp_ms;
        state->last_trip.trip_time_ms = timestamp_ms - state->ansi51.pickup_time_ms;
        state->trip_count++;
    }
    
    return trip;
}

void protection_reset(protection_state_t* state) {
    ansi50_reset(&state->ansi50);
    ansi51_reset(&state->ansi51);
}

void protection_get_last_trip(const protection_state_t* state, trip_event_t* event) {
    memcpy(event, &state->last_trip, sizeof(trip_event_t));
}
