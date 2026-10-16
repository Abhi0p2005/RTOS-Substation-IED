#ifndef TRIP_FSM_H
#define TRIP_FSM_H

#include <stdint.h>
#include <stdbool.h>
#include "protection/protection.h"

typedef enum {
    FSM_STATE_IDLE = 0,
    FSM_STATE_PICKUP,
    FSM_STATE_TRIP,
    FSM_STATE_LOCKOUT
} fsm_state_t;

typedef struct {
    fsm_state_t state;
    uint32_t state_entry_time_ms;
    trip_reason_t trip_reason;
    uint8_t faulted_phase;
    bool manual_reset_required;
} trip_fsm_t;

void trip_fsm_init(trip_fsm_t* fsm);
void trip_fsm_update(trip_fsm_t* fsm, bool trip_signal, trip_reason_t reason, uint8_t phase, uint32_t timestamp_ms);
void trip_fsm_reset(trip_fsm_t* fsm);
fsm_state_t trip_fsm_get_state(const trip_fsm_t* fsm);
const char* trip_fsm_get_state_name(fsm_state_t state);
bool trip_fsm_is_tripped(const trip_fsm_t* fsm);

#endif
