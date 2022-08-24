#include "mem.h"

ENTRY(_main)

MEMORY {
  FLASH   : ORIGIN = ADDR_FLASH, LENGTH = SIZE_FLASH
  RAM     : ORIGIN = ADDR_RAM, LENGTH = SIZE_RAM
}

SECTIONS
{
  .text ORIGIN(FLASH) : { boot.o(.text); *(.text) } > FLASH
  .rodata ORIGIN(RAM) : { *(.rodata) } > RAM
  .data : { *(.data) } > RAM
  .bss : { *(.bss COMMON) } > RAM
}
