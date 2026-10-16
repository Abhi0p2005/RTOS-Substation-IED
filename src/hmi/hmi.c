#include "hmi/hmi.h"

#ifdef ESP32_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "config.h"
#include "hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef NATIVE_SIM
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}
#endif

void hmi_init(void) {
#ifdef NATIVE_SIM
    enable_raw_mode();
#endif
    
    if (hal_lcd_available()) {
        hal_lcd_init();
        hal_lcd_clear();
        hal_lcd_print(0, 0, "IED Protection");
        hal_lcd_print(1, 0, "System Ready");
    }
}

void hmi_print_status(void) {
    printf("\n--- System Status ---\n");
    printf("ANSI 50: %s, Pickup: %.1f A\n",
        g_protection_config.ansi50.enabled ? "ON " : "OFF",
        g_protection_config.ansi50.pickup_current);
    printf("ANSI 51: %s, Pickup: %.1f A, TMS: %.2f\n",
        g_protection_config.ansi51.enabled ? "ON " : "OFF",
        g_protection_config.ansi51.pickup_current,
        g_protection_config.ansi51.time_multiplier);
    printf("Trip Relay: %s\n", hal_trip_relay_get() ? "CLOSED" : "OPEN");
    printf("--------------------\n");
}

void hmi_print_trip_history(void) {
    printf("\n--- Trip History ---\n");
    printf("Not implemented yet\n");
    printf("--------------------\n");
}

void hmi_update_lcd(void) {
    if (!hal_lcd_available()) {
        return;
    }
}

void hmi_task(void* params) {
    char cmd_buffer[128];
    uint8_t cmd_idx = 0;
    
    printf("\nHMI Commands:\n");
    printf("  status   - Show system status\n");
    printf("  config   - Show configuration\n");
    printf("  reset    - Reset trip condition\n");
    printf("  help     - Show this help\n");
#ifdef NATIVE_SIM
    printf("  fault <type> - Inject fault (none/overload/heavy/instant)\n");
    printf("  current <ia> <ib> <ic> - Set custom currents\n");
#endif
    printf("\n");
    
    while (1) {
#ifdef NATIVE_SIM
        int c = getchar();
        if (c != EOF && c != -1) {
            if (c == '\n' || c == '\r') {
                cmd_buffer[cmd_idx] = '\0';
                
                if (cmd_idx > 0) {
                    printf("\n");
                    
                    if (strcmp(cmd_buffer, "status") == 0) {
                        hmi_print_status();
                    }
                    else if (strcmp(cmd_buffer, "config") == 0) {
                        hmi_print_status();
                    }
                    else if (strcmp(cmd_buffer, "help") == 0) {
                        printf("Available commands: status, config, reset, help, fault, current\n");
                    }
                    else if (strncmp(cmd_buffer, "fault ", 6) == 0) {
                        char* fault_type = cmd_buffer + 6;
                        if (strcmp(fault_type, "none") == 0) {
                            hal_sim_inject_fault(FAULT_NONE, 0);
                            printf("Fault cleared\n");
                        }
                        else if (strcmp(fault_type, "overload") == 0) {
                            hal_sim_inject_fault(FAULT_OVERLOAD_150, 1.5f);
                            printf("Injected 150%% overload\n");
                        }
                        else if (strcmp(fault_type, "heavy") == 0) {
                            hal_sim_inject_fault(FAULT_HEAVY_300, 3.0f);
                            printf("Injected 300%% fault\n");
                        }
                        else if (strcmp(fault_type, "instant") == 0) {
                            hal_sim_inject_fault(FAULT_INSTANTANEOUS_800, 8.0f);
                            printf("Injected 800%% instantaneous fault\n");
                        }
                    }
                    else if (strncmp(cmd_buffer, "current ", 8) == 0) {
                        float ia, ib, ic;
                        if (sscanf(cmd_buffer + 8, "%f %f %f", &ia, &ib, &ic) == 3) {
                            hal_sim_set_current(ia, ib, ic);
                            printf("Set currents: Ia=%.2f, Ib=%.2f, Ic=%.2f\n", ia, ib, ic);
                        }
                    }
                    
                    printf("> ");
                    fflush(stdout);
                }
                
                cmd_idx = 0;
            }
            else if (c == 127 || c == 8) {
                if (cmd_idx > 0) {
                    cmd_idx--;
                    printf("\b \b");
                    fflush(stdout);
                }
            }
            else if (cmd_idx < sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_idx++] = c;
                printf("%c", c);
                fflush(stdout);
            }
        }
#endif
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
