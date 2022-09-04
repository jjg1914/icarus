#include "stm32f100xb.h"
#include "scheduler.h"
#include "usart.h"

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

void init()
{
  enable_clocks();

  NVIC_SetPriorityGrouping(0);
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(0, 1, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  usart_set_baud_rate(USART1, 9600);
  usart_enable(USART1);
  usart_write(USART1, "Hello, world!\n");
  while (1) {
    uint8_t c;
    usart_getc(USART1, &c);
    usart_putc(USART1, c);
  }
  __asm__("SVC #0");
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
