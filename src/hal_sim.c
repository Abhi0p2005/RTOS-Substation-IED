#include "hal.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef NATIVE_SIM

static fault_scenario_t current_fault = FAULT_NONE;
static float fault_magnitude = 0.0f;
static float custom_ia = 0.0f, custom_ib = 0.0f, custom_ic = 0.0f;
static bool trip_relay_state = false;
static uint32_t sim_time_ms = 0;
static struct timespec start_time;

#define PI 3.14159265358979323846

void hal_init(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    trip_relay_state = false;
    sim_time_ms = 0;
    printf("[HAL-SIM] Initialized\n");
}

void hal_adc_start(void) {
    printf("[HAL-SIM] ADC started\n");
}

void hal_adc_stop(void) {
    printf("[HAL-SIM] ADC stopped\n");
}

bool hal_adc_read(sample_data_t* data) {
    static uint32_t sample_count = 0;
    
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    
    uint32_t elapsed_sec = current_time.tv_sec - start_time.tv_sec;
    uint32_t elapsed_nsec = current_time.tv_nsec - start_time.tv_nsec;
    sim_time_ms = (elapsed_sec * 1000) + (elapsed_nsec / 1000000);
    
    float time_sec = (float)sim_time_ms / 1000.0f;
    float angle = 2.0f * PI * SYSTEM_FREQUENCY_HZ * time_sec;
    
    float ia_peak = RATED_CURRENT_A * 1.414f;
    float ib_peak = RATED_CURRENT_A * 1.414f;
    float ic_peak = RATED_CURRENT_A * 1.414f;
    
    switch (current_fault) {
        case FAULT_OVERLOAD_150:
            ia_peak = 1.5f * RATED_CURRENT_A * 1.414f;
            ib_peak = 1.5f * RATED_CURRENT_A * 1.414f;
            ic_peak = 1.5f * RATED_CURRENT_A * 1.414f;
            break;
        case FAULT_HEAVY_300:
            ia_peak = 3.0f * RATED_CURRENT_A * 1.414f;
            ib_peak = 3.0f * RATED_CURRENT_A * 1.414f;
            ic_peak = 3.0f * RATED_CURRENT_A * 1.414f;
            break;
        case FAULT_INSTANTANEOUS_800:
            ia_peak = 8.0f * RATED_CURRENT_A * 1.414f;
            ib_peak = 8.0f * RATED_CURRENT_A * 1.414f;
            ic_peak = 8.0f * RATED_CURRENT_A * 1.414f;
            break;
        case FAULT_CUSTOM:
            ia_peak = custom_ia * 1.414f;
            ib_peak = custom_ib * 1.414f;
            ic_peak = custom_ic * 1.414f;
            break;
        case FAULT_NONE:
        default:
            break;
    }
    
    data->ia = ia_peak * sinf(angle);
    data->ib = ib_peak * sinf(angle - 2.0f * PI / 3.0f);
    data->ic = ic_peak * sinf(angle + 2.0f * PI / 3.0f);
    
    data->va = 230.0f * 1.414f * sinf(angle);
    data->vb = 230.0f * 1.414f * sinf(angle - 2.0f * PI / 3.0f);
    data->vc = 230.0f * 1.414f * sinf(angle + 2.0f * PI / 3.0f);
    
    data->timestamp_ms = sim_time_ms;
    
    sample_count++;
    return true;
}

void hal_trip_relay_set(bool state) {
    if (trip_relay_state != state) {
        trip_relay_state = state;
        printf("[HAL-SIM] Trip relay: %s\n", state ? "CLOSED (TRIP)" : "OPEN");
    }
}

bool hal_trip_relay_get(void) {
    return trip_relay_state;
}

void hal_led_set(uint8_t led_id, bool state) {
}

uint32_t hal_get_timestamp_ms(void) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    
    uint32_t elapsed_sec = current_time.tv_sec - start_time.tv_sec;
    uint32_t elapsed_nsec = current_time.tv_nsec - start_time.tv_nsec;
    return (elapsed_sec * 1000) + (elapsed_nsec / 1000000);
}

void hal_sim_inject_fault(fault_scenario_t fault, float magnitude) {
    current_fault = fault;
    fault_magnitude = magnitude;
    printf("[HAL-SIM] Fault injected: %d, magnitude: %.2f\n", fault, magnitude);
}

void hal_sim_set_current(float ia, float ib, float ic) {
    custom_ia = ia;
    custom_ib = ib;
    custom_ic = ic;
    current_fault = FAULT_CUSTOM;
}

void hal_lcd_init(void) {
}

void hal_lcd_clear(void) {
}

void hal_lcd_print(uint8_t row, uint8_t col, const char* text) {
}

bool hal_lcd_available(void) {
    return false;
}

#endif
