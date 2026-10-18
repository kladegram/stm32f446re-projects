#pragma once

#include <stdint.h>

void usart_init(void);
void usart_write_char(char c);
void usart_write_string(const char *s);
