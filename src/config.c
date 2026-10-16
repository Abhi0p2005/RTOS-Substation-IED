#include "config.h"
#include "protection/ansi51.h"
#include <string.h>

protection_config_t g_protection_config;

void config_init(void) {
    config_load_defaults();
}

void config_load_defaults(void) {
    g_protection_config.rated_current = RATED_CURRENT_A;
    g_protection_config.system_frequency = SYSTEM_FREQUENCY_HZ;
    
    g_protection_config.ansi50.enabled = true;
    g_protection_config.ansi50.pickup_current = 8.0f * RATED_CURRENT_A;
    g_protection_config.ansi50.delay_ms = 50;
    
    g_protection_config.ansi51.enabled = true;
    g_protection_config.ansi51.pickup_current = 1.5f * RATED_CURRENT_A;
    g_protection_config.ansi51.time_multiplier = 0.1f;
    g_protection_config.ansi51.curve_type = CURVE_IEC_STANDARD_INVERSE;
}

void config_save_to_flash(void) {
#ifdef ESP32_TARGET
    
#endif
}

void config_load_from_flash(void) {
#ifdef ESP32_TARGET
    
#else
    config_load_defaults();
#endif
}
