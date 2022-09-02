#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "stdint.h"

typedef void (*scheduler_function)(void);

void scheduler_init();
int scheduler_exec(scheduler_function entry);
void scheduler_wait(uint8_t tid);
void scheduler_yield();

#endif
