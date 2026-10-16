#include "hal.h"

#ifdef ESP32_TARGET

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAG "HAL-ESP32"

#define ADC_IA_CHANNEL      ADC1_CHANNEL_0
#define ADC_IB_CHANNEL      ADC1_CHANNEL_3
#define ADC_IC_CHANNEL      ADC1_CHANNEL_6

#define GPIO_TRIP_RELAY     GPIO_NUM_25
#define GPIO_LED_STATUS     GPIO_NUM_2
#define GPIO_LED_TRIP       GPIO_NUM_4

static bool trip_relay_state = false;

void hal_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_TRIP_RELAY) | (1ULL << GPIO_LED_STATUS) | (1ULL << GPIO_LED_TRIP),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    gpio_set_level(GPIO_TRIP_RELAY, 0);
    gpio_set_level(GPIO_LED_STATUS, 0);
    gpio_set_level(GPIO_LED_TRIP, 0);
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_IA_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_IB_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_IC_CHANNEL, ADC_ATTEN_DB_11);
    
    trip_relay_state = false;
    
    ESP_LOGI(TAG, "HAL initialized");
}

void hal_adc_start(void) {
    ESP_LOGI(TAG, "ADC started");
}

void hal_adc_stop(void) {
    ESP_LOGI(TAG, "ADC stopped");
}

bool hal_adc_read(sample_data_t* data) {
    int adc_ia = adc1_get_raw(ADC_IA_CHANNEL);
    int adc_ib = adc1_get_raw(ADC_IB_CHANNEL);
    int adc_ic = adc1_get_raw(ADC_IC_CHANNEL);
    
    data->ia = ((float)adc_ia - (ADC_MAX_VALUE / 2.0f)) * ADC_CURRENT_SCALE * CT_RATIO;
    data->ib = ((float)adc_ib - (ADC_MAX_VALUE / 2.0f)) * ADC_CURRENT_SCALE * CT_RATIO;
    data->ic = ((float)adc_ic - (ADC_MAX_VALUE / 2.0f)) * ADC_CURRENT_SCALE * CT_RATIO;
    
    data->va = 0.0f;
    data->vb = 0.0f;
    data->vc = 0.0f;
    
    data->timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000);
    
    return true;
}

void hal_trip_relay_set(bool state) {
    trip_relay_state = state;
    gpio_set_level(GPIO_TRIP_RELAY, state ? 1 : 0);
    gpio_set_level(GPIO_LED_TRIP, state ? 1 : 0);
    ESP_LOGI(TAG, "Trip relay: %s", state ? "CLOSED" : "OPEN");
}

bool hal_trip_relay_get(void) {
    return trip_relay_state;
}

void hal_led_set(uint8_t led_id, bool state) {
    if (led_id == 0) {
        gpio_set_level(GPIO_LED_STATUS, state ? 1 : 0);
    }
}

uint32_t hal_get_timestamp_ms(void) {
    return (uint32_t)(esp_timer_get_time() / 1000);
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
