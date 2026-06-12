#ifndef CUP_STALKER_SCHEDULER_H
#define CUP_STALKER_SCHEDULER_H

#include <stdbool.h>

void scheduler_start(int interval_seconds);
void scheduler_stop(void);
bool scheduler_poll(void);
void scheduler_trigger_now(void);

#endif
