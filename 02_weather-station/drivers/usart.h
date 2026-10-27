#pragma once

#include <stdbool.h>
#include <stdint.h>

void usart_init(void);
void usart_write_char(char c);
void usart_write_string(const char *s);
void usart_write_int(int i);
void usart_write_float(float f);
bool usart_read_char(char *out_char);
