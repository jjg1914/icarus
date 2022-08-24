#include "mem.h"

void reset(void) {
  MEM_WRITE_REG(ADDR_AIRCR, (VECTKEY | SYSRESETREQ));
}
 
void _main() {
  reset();
}
