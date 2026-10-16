#include <stdio.h>
#include <string.h>

#ifdef ESP32_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif

#include "hal.h"
#include "config.h"
#include "rms.h"
#include "protection/protection.h"
#include "fsm/trip_fsm.h"
#include "hmi/hmi.h"

#define ADC_TASK_PRIORITY       4
#define RMS_TASK_PRIORITY       3
#define PROTECTION_TASK_PRIORITY 2
#define HMI_TASK_PRIORITY       1

#define ADC_QUEUE_SIZE          32
#define RMS_QUEUE_SIZE          8

static QueueHandle_t adc_queue;
static QueueHandle_t rms_queue;

typedef struct {
    float ia_rms;
    float ib_rms;
    float ic_rms;
    uint32_t timestamp_ms;
} rms_result_t;

static protection_state_t g_protection_state;
static trip_fsm_t g_trip_fsm;

void adc_task(void* params) {
    sample_data_t sample;
    
    hal_adc_start();
    
    const TickType_t sample_period = pdMS_TO_TICKS(1000 / SAMPLING_RATE_HZ);
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        if (hal_adc_read(&sample)) {
            xQueueSend(adc_queue, &sample, 0);
        }
        
        vTaskDelayUntil(&last_wake_time, sample_period);
    }
}

void rms_task(void* params) {
    rms_calculator_t rms_ia, rms_ib, rms_ic;
    rms_init(&rms_ia);
    rms_init(&rms_ib);
    rms_init(&rms_ic);
    
    sample_data_t sample;
    rms_result_t result;
    
    while (1) {
        if (xQueueReceive(adc_queue, &sample, portMAX_DELAY) == pdTRUE) {
            bool ia_ready = rms_update(&rms_ia, sample.ia);
            bool ib_ready = rms_update(&rms_ib, sample.ib);
            bool ic_ready = rms_update(&rms_ic, sample.ic);
            
            if (ia_ready && ib_ready && ic_ready) {
                result.ia_rms = rms_get_value(&rms_ia);
                result.ib_rms = rms_get_value(&rms_ib);
                result.ic_rms = rms_get_value(&rms_ic);
                result.timestamp_ms = sample.timestamp_ms;
                
                xQueueSend(rms_queue, &result, 0);
            }
        }
    }
}

void protection_task(void* params) {
    rms_result_t rms_data;
    trip_reason_t reason;
    
    while (1) {
        if (xQueueReceive(rms_queue, &rms_data, portMAX_DELAY) == pdTRUE) {
            bool trip = protection_evaluate(
                &g_protection_state,
                rms_data.ia_rms,
                rms_data.ib_rms,
                rms_data.ic_rms,
                rms_data.timestamp_ms,
                &reason
            );
            
            uint8_t faulted_phase = 0;
            if (reason == TRIP_REASON_ANSI50) {
                faulted_phase = ansi50_get_faulted_phase(&g_protection_state.ansi50);
            } else if (reason == TRIP_REASON_ANSI51) {
                faulted_phase = ansi51_get_faulted_phase(&g_protection_state.ansi51);
            }
            
            trip_fsm_update(&g_trip_fsm, trip, reason, faulted_phase, rms_data.timestamp_ms);
            
            if (trip_fsm_is_tripped(&g_trip_fsm)) {
                hal_trip_relay_set(true);
                hal_led_set(1, true);
                
                printf("\n=== TRIP EVENT ===\n");
                printf("Reason: %s\n", (reason == TRIP_REASON_ANSI50) ? "ANSI 50 (Instantaneous)" : "ANSI 51 (Time Overcurrent)");
                printf("Phase: %c\n", 'A' + faulted_phase - 1);
                printf("Ia RMS: %.3f A\n", rms_data.ia_rms);
                printf("Ib RMS: %.3f A\n", rms_data.ib_rms);
                printf("Ic RMS: %.3f A\n", rms_data.ic_rms);
                printf("Time: %u ms\n", (unsigned int)rms_data.timestamp_ms);
                printf("==================\n\n");
            } else {
                hal_trip_relay_set(false);
                hal_led_set(1, false);
            }
        }
    }
}

int main(void) {
    printf("\n");
    printf("================================================\n");
    printf("  IED Protection System - ESP32 FreeRTOS\n");
    printf("  50Hz System | ANSI 50/51 Protection\n");
    printf("================================================\n\n");
    
    config_init();
    hal_init();
    
    printf("Configuration:\n");
    printf("  ANSI 50: %s, Pickup: %.1f A, Delay: %u ms\n",
        g_protection_config.ansi50.enabled ? "Enabled" : "Disabled",
        g_protection_config.ansi50.pickup_current,
        (unsigned int)g_protection_config.ansi50.delay_ms);
    printf("  ANSI 51: %s, Pickup: %.1f A, TMS: %.2f\n\n",
        g_protection_config.ansi51.enabled ? "Enabled" : "Disabled",
        g_protection_config.ansi51.pickup_current,
        g_protection_config.ansi51.time_multiplier);
    
    protection_init(&g_protection_state);
    trip_fsm_init(&g_trip_fsm);
    
    adc_queue = xQueueCreate(ADC_QUEUE_SIZE, sizeof(sample_data_t));
    rms_queue = xQueueCreate(RMS_QUEUE_SIZE, sizeof(rms_result_t));
    
    xTaskCreate(adc_task, "ADC", 16384, NULL, ADC_TASK_PRIORITY, NULL);
    xTaskCreate(rms_task, "RMS", 16384, NULL, RMS_TASK_PRIORITY, NULL);
    xTaskCreate(protection_task, "PROT", 16384, NULL, PROTECTION_TASK_PRIORITY, NULL);
    xTaskCreate(hmi_task, "HMI", 16384, NULL, HMI_TASK_PRIORITY, NULL);
    
    hal_led_set(0, true);
    
    printf("System started. Monitoring...\n\n");
    
    vTaskStartScheduler();
    
    while (1) {
    }
    
    return 0;
}
