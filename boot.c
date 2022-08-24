#include "mem.h"

void _main();

__attribute__ ((section (".text"))) void* _vector[LEN_VECTOR] = {
   (void*) ADDR_INIT_STACK,
   _main
};
