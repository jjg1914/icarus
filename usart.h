#ifndef _USART_H_
#define _USART_H_

#include "stm32f100xb.h"

void usart_enable(USART_TypeDef* usart);
void usart_set_baud_rate(USART_TypeDef* usart, uint32_t baud);
void usart_putc(USART_TypeDef* usart, uint8_t c);
void usart_getc(USART_TypeDef* usart, uint8_t* c);
void usart_write(USART_TypeDef* usart, const char* buffer);

#endif
