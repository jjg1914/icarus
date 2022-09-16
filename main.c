#include "stm32f100xb.h"
#include "scheduler.h"
#include "usart.h"
#include "util.h"

void reset(void)
{
  SCB->AIRCR = ((0x05FAU << SCB_AIRCR_VECTKEY_Pos) & SCB_AIRCR_VECTKEYSTAT_Msk) |
    SCB_AIRCR_SYSRESETREQ_Msk;
}

void enable_clocks()
{
  // Enable peripheral clocks: GPIOA, USART2.
  //RCC->APB1ENR |= (RCC_APB1ENR_USART2EN);
  RCC->APB2ENR |= (RCC_APB2ENR_USART1EN); // enable USART1 on the high speed domain
  RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN); // enable IO Port A on the high speed domain
  // Configure pins A2, A3 for USART2.
  GPIOA->CRL &= (GPIO_CRL_MODE2 |
                 GPIO_CRL_CNF2 |
                 GPIO_CRL_MODE3 |
                 GPIO_CRL_CNF3);
  GPIOA->CRL |= ((0x1 << GPIO_CRL_MODE2_Pos) |
                 (0x2 << GPIO_CRL_CNF2_Pos) |
                 (0x0 << GPIO_CRL_MODE3_Pos) |
                 (0x1 << GPIO_CRL_CNF3_Pos));
}

static struct {
  uint8_t tasks_total;
  uint8_t tasks_counter;
  volatile int counter;
  int limit;
} test_1_data = { 0, 0, 0, 0 };

void test_1()
{
  uint8_t task_number = test_1_data.tasks_counter++;
  while (test_1_data.counter < test_1_data.limit) {
    while (test_1_data.counter % test_1_data.tasks_total != task_number) {
      scheduler_yield();
    }
    test_1_data.counter += 1;
  }
}

void test(char* args)
{
  int test_number;
  atoi(strtok(args, " "), &test_number);

  switch (test_number) {
    case 1:
      {
        int tmp;
        atoi(strtok(args, " "), &tmp);
        test_1_data.tasks_total = tmp;
        if (test_1_data.tasks_total > 8) {
          test_1_data.tasks_total = 8;
        }

        atoi(strtok(args, " "), &tmp);
        test_1_data.limit = tmp;

        test_1_data.tasks_counter = 0;
        test_1_data.counter = 0;
      }

      {
        uint8_t tasks[8];
        for (uint8_t i = 0; i < test_1_data.tasks_total; i += 1) {
          tasks[i] = scheduler_exec(test_1);
        }
        for (uint8_t i = 0; i < test_1_data.tasks_total; i += 1) {
          scheduler_wait(tasks[i]);
        }
      }

      {
        uint8_t buf[12];
        uint32_t len = fmti(buf, test_1_data.counter);
        buf[len] = '\0';
        usart_write(USART1, buf);
        usart_putc(USART1, '\n');
      }
  }
}

void init()
{
  enable_clocks();

  NVIC_SetPriorityGrouping(0);
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(0, 1, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  usart_set_baud_rate(USART1, 9600);
  usart_enable(USART1);

  while (1) {
    usart_putc(USART1, '>');

    uint8_t buf[64], *bufp = buf;

    uint8_t flag = 0;
    do {
      uint8_t c;
      usart_getc(USART1, &c);

      switch (c) {
        case '\r':
        case '\n':
          usart_putc(USART1, c == '\r' ? '\n' : c);
          *bufp = '\0';
          flag = 1;
          break;
        case '\b':
          if (bufp > buf) {
            usart_putc(USART1, c);
            bufp--;
          }
          break;
        default:
          if ((bufp - buf) < (sizeof(buf) - 1)) {
            usart_putc(USART1, c);
            *(bufp++) = c;
          }
          break;
      }
    } while (!flag);

    switch (buf[0]) {
      case '$':
        {
          uint32_t len = fmti(buf, scheduler_tid());
          buf[len] = '\0';
          usart_write(USART1, buf);
          usart_putc(USART1, '\n');
        }
        break;
      case 't':
        test(buf + 1);
        break;
      case 'x':
        __asm__("SVC #0");
        break;
    }
  }
}

void main()
{
  scheduler_init();
  scheduler_exec(init);
  scheduler_yield();
}

void __attribute__ ((naked)) SVC_Handler()
{
  register uint8_t** r0 __asm("r0");

  __asm__(
    "TST lr, #4\n"
    "ITE EQ\n"
    "MRSEQ r0, MSP\n"
    "MRSNE r0, PSP\n"
  );

  uint8_t svc_number = r0[6][-2];
  switch (svc_number) {
    case 0:
      reset();
      break;
  }
}

void __libc_init_array()
{
}
