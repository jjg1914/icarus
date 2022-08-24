#define ADDR_FLASH (0x08000000)
#define SIZE_FLASH (0x400 * 128)
#define ADDR_RAM (0x08000000)
#define SIZE_RAM (0x400 * 128)

#define ADDR_VECTOR (ADDR_FLASH)
#define LEN_VECTOR (1024)

#define ADDR_INIT_STACK (0x20001000)
#define ADDR_AIRCR (0xE000ED0C)

#define SYSRESETREQ (0x0004)
#define VECTKEY (0x05FA0000)

#define MEM_WRITE_REG(R, V) \
  ((*(volatile unsigned int *) R) = V)
