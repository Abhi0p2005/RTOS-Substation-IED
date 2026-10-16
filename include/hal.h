#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FAULT_NONE = 0,
    FAULT_OVERLOAD_150,
    FAULT_HEAVY_300,
    FAULT_INSTANTANEOUS_800,
    FAULT_CUSTOM
} fault_scenario_t;

typedef struct {
    float ia;
    float ib;
    float ic;
    float va;
    float vb;
    float vc;
    uint32_t timestamp_ms;
} sample_data_t;

void hal_init(void);
void hal_adc_start(void);
void hal_adc_stop(void);
bool hal_adc_read(sample_data_t* data);
void hal_trip_relay_set(bool state);
bool hal_trip_relay_get(void);
void hal_led_set(uint8_t led_id, bool state);
uint32_t hal_get_timestamp_ms(void);

#ifdef NATIVE_SIM
void hal_sim_inject_fault(fault_scenario_t fault, float magnitude);
void hal_sim_set_current(float ia, float ib, float ic);
#endif

#ifdef ESP32_TARGET
void hal_esp32_adc_init(void);
#endif

void hal_lcd_init(void);
void hal_lcd_clear(void);
void hal_lcd_print(uint8_t row, uint8_t col, const char* text);
bool hal_lcd_available(void);

#endif
