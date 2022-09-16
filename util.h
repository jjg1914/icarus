#ifndef _UTIL_H_
#define _UTIL_H_

#include "stdint.h"

#define RING_BUFFER_LEN (32)

typedef struct ring_buffer_t {
  volatile uint32_t head;
  volatile uint32_t tail;
  volatile char buffer[RING_BUFFER_LEN];
} ring_buffer_t;

#define RING_BUFFER_DEFAULT { 0, 0 }
#define RING_BUFFER_EMPTY(X) ((X)->head == (X)->tail)
#define RING_BUFFER_FULL(X) \
  (((X)->tail == 0 ? (X)->head == sizeof((X)->buffer) - 1 : (X)->head == (X)->tail - 1))
#define RING_BUFFER_ADVANCE(X, PTR) \
  ((++(X)->PTR) == sizeof((X)->buffer) ? (X)->PTR = 0 : (X)->PTR)
#define RING_BUFFER_PEEK(X) ((X)->buffer[(X)->tail])
#define RING_BUFFER_PUT(X, C) ((X)->buffer[(X)->head] = C)

uint32_t fmti(uint8_t* dest, int i);
uint32_t atoi(uint8_t* src, int* i);
char* strtok(uint8_t * str, const uint8_t * delimeters);

#endif
