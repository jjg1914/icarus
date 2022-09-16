#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "stdint.h"

typedef void (*scheduler_function)(void);
typedef enum task_state_e {
  E_TASK_STATE_OPEN = 0,
  E_TASK_STATE_DEFUNCT,
  E_TASK_STATE_PENDING,
  E_TASK_STATE_BLOCK,
} task_state_e;

typedef enum task_block_e {
  E_TASK_BLOCK_NONE = 0,
  E_TASK_BLOCK_USART_RX,
  E_TASK_BLOCK_USART_TXE,
} task_block_e;

void scheduler_init();
int scheduler_exec(scheduler_function entry);
void scheduler_wait(uint8_t tid);
void scheduler_yield();
void scheduler_block(task_block_e block);
void scheduler_unblock(task_block_e block);
task_state_e scheduler_stat(uint8_t tid);
uint8_t scheduler_tid();

#endif
