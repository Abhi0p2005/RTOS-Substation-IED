#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define SYSTEM_FREQUENCY_HZ         50
#define SAMPLES_PER_CYCLE           128
#define SAMPLING_RATE_HZ            (SYSTEM_FREQUENCY_HZ * SAMPLES_PER_CYCLE)
#define RATED_CURRENT_A             5.0f
#define CT_PRIMARY_A                100.0f
#define CT_SECONDARY_A              5.0f
#define CT_RATIO                    (CT_PRIMARY_A / CT_SECONDARY_A)

#define RMS_WINDOW_CYCLES           1
#define RMS_WINDOW_SAMPLES          (SAMPLES_PER_CYCLE * RMS_WINDOW_CYCLES)

#define ADC_RESOLUTION_BITS         12
#define ADC_MAX_VALUE               ((1 << ADC_RESOLUTION_BITS) - 1)
#define ADC_VREF_V                  3.3f
#define ADC_BURDEN_RESISTOR_OHM     0.66f
#define ADC_CURRENT_SCALE           (ADC_VREF_V / (ADC_MAX_VALUE * ADC_BURDEN_RESISTOR_OHM))

typedef struct {
    float pickup_current;
    uint32_t delay_ms;
    bool enabled;
} ansi50_config_t;

typedef struct {
    float pickup_current;
    float time_multiplier;
    uint8_t curve_type;
    bool enabled;
} ansi51_config_t;

typedef struct {
    ansi50_config_t ansi50;
    ansi51_config_t ansi51;
    float rated_current;
    uint8_t system_frequency;
} protection_config_t;

extern protection_config_t g_protection_config;

void config_init(void);
void config_load_defaults(void);
void config_save_to_flash(void);
void config_load_from_flash(void);

#endif
