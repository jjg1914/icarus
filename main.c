#include "ARMCM3.h"

void reset(void) {
  SCB->AIRCR = ((0x05FAU << SCB_AIRCR_VECTKEY_Pos) & SCB_AIRCR_VECTKEYSTAT_Msk) |
    SCB_AIRCR_SYSRESETREQ_Msk;
}

void _SVC_Handler(unsigned int* svc_args)
{
  unsigned int svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
}

void _main() {
  __asm__("SVC 0");
  reset();
}

void __attribute__ ((naked)) SVC_Handler() {
  __asm__(
    "TST lr, #4\n"
    "ITE EQ\n"
    "MRSEQ r0, MSP\n"
    "MRSNE r0, PSP\n"
    "B _SVC_Handler\n"
  );
}
