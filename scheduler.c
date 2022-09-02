#include "scheduler.h"

#include "stm32f100xb.h"

#define TASK_STACK_SIZE (256)
#define IDLE_TASK_STACK_SIZE (128)
#define TASKS_MAX (8)

extern uint32_t __INITIAL_SP;
extern uint32_t __LIMIT_SP;

typedef enum task_state_e {
  E_TASK_STATE_OPEN = 0,
  E_TASK_STATE_DEFUNCT,
  E_TASK_STATE_PENDING,
} task_state_e;

typedef struct task_t {
  task_state_e state;
  void* stack;
  void* stackp;
} task_t;

typedef struct __attribute__((__packed__)) stack_frame_t {
 	// Registers that the software has to push on the stack, after the
 	// NVIC has pushed some specific registers.
 	// Corresponds to registers pushed by "STMDB %0!, {r4-r11}".
 	struct {
 		uint32_t registers[8];
 	} software_frame;

 	// Registers pushed on the stack by NVIC (hardware), before the other
 	// registers defined above.
 	struct {
 		uint32_t r0;
 		uint32_t r1;
 		uint32_t r2;
 		uint32_t r3;
 		uint32_t r12;
 		void *lr;
 		void *pc;
 		uint32_t psr;
 	} nvic_frame;
  uint8_t padding[3];
} stack_frame_t;

static uint8_t __attribute__((aligned(8))) stacks[TASKS_MAX * TASK_STACK_SIZE];
static uint8_t __attribute__((aligned(8))) idle_stack[IDLE_TASK_STACK_SIZE];
static task_t tasks[TASKS_MAX];
static task_t idle_task;
static uint8_t current_task = 0;

static void scheduler_return()
{
  tasks[current_task].state = E_TASK_STATE_DEFUNCT;
  scheduler_yield();
}

static void scheduler_idle_task()
{
  while(1) {
    __WFI();
    scheduler_yield();
  }
}

static void scheduler_task_init(task_t* task, scheduler_function entry, uint32_t stack_size)
{
  // Set stack pointer to beginning of stack
  task->stackp = task->stack + stack_size - 1;

  // Push the same thing that a PendSV would push on the task's
  // stack, with dummy values for the general purpose registers.
  task->stackp -= sizeof(stack_frame_t);
  stack_frame_t *frame = (stack_frame_t*) task->stackp;
  frame->nvic_frame.r0 = 0xff00ff00;
  frame->nvic_frame.r1 = 0xff00ff01;
  frame->nvic_frame.r2 = 0xff00ff02;
  frame->nvic_frame.r3 = 0xff00ff03;
  frame->nvic_frame.r12 = 0xff00ff0c;
  frame->nvic_frame.lr = scheduler_return;
  frame->nvic_frame.pc = entry;
  frame->nvic_frame.psr = 0x21000000; /* Default, allegedly */
  frame->software_frame.registers[0] = 0xff00ff04;
  frame->software_frame.registers[1] = 0xff00ff05;
  frame->software_frame.registers[2] = 0xff00ff06;
  frame->software_frame.registers[3] = 0xff00ff07;
  frame->software_frame.registers[4] = 0xff00ff08;
  frame->software_frame.registers[5] = 0xff00ff09;
  frame->software_frame.registers[6] = 0xff00ff0a;
  frame->software_frame.registers[7] = 0xff00ff0b;

  task->state = E_TASK_STATE_PENDING;
}

void scheduler_init()
{
  for (int i = 0; i < TASKS_MAX; i += 1) {
      tasks[i].state = E_TASK_STATE_OPEN;
      tasks[i].stack = stacks + (i * TASK_STACK_SIZE);
      tasks[i].stackp = tasks[i].stack + TASK_STACK_SIZE - 1;
  }

  idle_task.stack = idle_stack;
  scheduler_task_init(&idle_task, scheduler_idle_task, IDLE_TASK_STACK_SIZE);
}

void scheduler_wait(uint8_t tid)
{
  while (tasks[tid].state != E_TASK_STATE_DEFUNCT) {
    scheduler_yield();
  }
  tasks[tid].state = E_TASK_STATE_OPEN;
}

int scheduler_exec(scheduler_function entry)
{
  int i;
  for (i = 0; i < TASKS_MAX; i += 1) {
    if (tasks[i].state == E_TASK_STATE_OPEN) {
      break;
    }
  }

  if (i >= TASKS_MAX) {
    return -1;
  }

  task_t *task = &tasks[i];

  scheduler_task_init(task, entry, TASK_STACK_SIZE);

  return i;
}

void scheduler_yield()
{
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
  __DSB();
  __ISB();
}

void __attribute__ ((naked)) PendSV_Handler()
{
  register int lr __asm("lr");
  register uint8_t* r0 __asm("r0");

  task_t* task = &tasks[current_task];

  if (lr & 0x4) {
    __asm__(
 			"MRS r0, psp\n"
 			"STMDB r0!, {r4-r11}\n"
    );
    tasks[current_task].stackp = r0;

    uint8_t old_task = current_task;
    do {
      current_task = (current_task + 1) % TASKS_MAX;
      task = &tasks[current_task];
    } while (current_task != old_task &&
        tasks[current_task].state != E_TASK_STATE_PENDING);

    if (current_task == old_task && task->state != E_TASK_STATE_PENDING) {
      task = &idle_task;
    }
  }

  r0 = task->stackp;
  __asm__(
    "LDMFD r0!, {r4-r11}\n"
    "MSR psp, r0\n"
    "LDR r0,=0xfffffffd\n"
    "BX r0\n"
  );
}
