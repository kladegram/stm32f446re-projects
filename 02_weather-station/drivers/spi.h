#pragma once

#include "./timer.h"
#include <stdbool.h>
#include <stdint.h>

void spi2_init(void);
bool spi2_write(unsigned char);