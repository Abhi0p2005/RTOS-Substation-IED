#include "fsm/trip_fsm.h"
#include <string.h>

void trip_fsm_init(trip_fsm_t* fsm) {
    fsm->state = FSM_STATE_IDLE;
    fsm->state_entry_time_ms = 0;
    fsm->trip_reason = TRIP_REASON_NONE;
    fsm->faulted_phase = 0;
    fsm->manual_reset_required = false;
}

void trip_fsm_update(trip_fsm_t* fsm, bool trip_signal, trip_reason_t reason, uint8_t phase, uint32_t timestamp_ms) {
    fsm_state_t prev_state = fsm->state;
    
    switch (fsm->state) {
        case FSM_STATE_IDLE:
            if (trip_signal) {
                fsm->state = FSM_STATE_TRIP;
                fsm->trip_reason = reason;
                fsm->faulted_phase = phase;
                fsm->state_entry_time_ms = timestamp_ms;
            }
            break;
            
        case FSM_STATE_PICKUP:
            if (trip_signal) {
                fsm->state = FSM_STATE_TRIP;
                fsm->trip_reason = reason;
                fsm->faulted_phase = phase;
                fsm->state_entry_time_ms = timestamp_ms;
            } else {
                fsm->state = FSM_STATE_IDLE;
                fsm->state_entry_time_ms = timestamp_ms;
            }
            break;
            
        case FSM_STATE_TRIP:
            fsm->manual_reset_required = true;
            break;
            
        case FSM_STATE_LOCKOUT:
            break;
    }
    
    if (prev_state != fsm->state) {
        fsm->state_entry_time_ms = timestamp_ms;
    }
}

void trip_fsm_reset(trip_fsm_t* fsm) {
    if (fsm->state == FSM_STATE_TRIP || fsm->state == FSM_STATE_LOCKOUT) {
        fsm->state = FSM_STATE_IDLE;
        fsm->trip_reason = TRIP_REASON_NONE;
        fsm->faulted_phase = 0;
        fsm->manual_reset_required = false;
    }
}

fsm_state_t trip_fsm_get_state(const trip_fsm_t* fsm) {
    return fsm->state;
}

const char* trip_fsm_get_state_name(fsm_state_t state) {
    switch (state) {
        case FSM_STATE_IDLE: return "IDLE";
        case FSM_STATE_PICKUP: return "PICKUP";
        case FSM_STATE_TRIP: return "TRIP";
        case FSM_STATE_LOCKOUT: return "LOCKOUT";
        default: return "UNKNOWN";
    }
}

bool trip_fsm_is_tripped(const trip_fsm_t* fsm) {
    return (fsm->state == FSM_STATE_TRIP || fsm->state == FSM_STATE_LOCKOUT);
}
