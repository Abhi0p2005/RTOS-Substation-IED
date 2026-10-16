#ifndef HMI_H
#define HMI_H

#include <stdint.h>
#include <stdbool.h>

void hmi_init(void);
void hmi_task(void* params);
void hmi_print_status(void);
void hmi_print_trip_history(void);
void hmi_update_lcd(void);

#endif
