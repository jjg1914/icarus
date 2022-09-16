#include "util.h"

uint32_t fmti(uint8_t* dest, int i)
{
  if (i == 0) {
    dest[0] = '0';
    return 1;
  }

  uint32_t rval = 0;
  uint8_t* p = dest;
  while (i != 0) {
    int rem = i % 10;
    i = i / 10;
    *(p++) = '0' + rem;
    rval += 1;
  }
  --p;

  while (p > dest) {
    char c = *dest;
    *(dest++) = *p;
    *(p--) = c;
  }

  return rval;
}

uint32_t atoi(uint8_t* src, int* i)
{
  uint32_t rval = 0;
  int tmp = 0;

  while (*src >= '0' && *src <= '9') {
    tmp = tmp * 10 + (*(src++) - '0');
    rval += 1;
  }

  *i = tmp;
  return rval;
}

char* strtok(uint8_t * str, const uint8_t * delimeters)
{
  static uint8_t* cur = 0, *curp;
  if (str != cur) {
    cur = str;
    curp = str;
  }

  if (!*curp) {
    return 0;
  }

  uint8_t* rval = curp;
  while (*curp) {
    for (const uint8_t* c = delimeters; *c; c += 1) {
      if (*curp == *c) {
        *curp = '\0';
        curp += 1;
        goto end_loop;
      }
    }
    curp += 1;
  };
end_loop:

  return rval;
}
