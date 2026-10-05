# STM32F446RE Projects

Embedded C projects on the **ST NUCLEO-F446RE** board (Arm Cortex-M4F, 180 MHz, 512 KB flash, 128 KB RAM).

This repository is my embedded-programming learning path. Each project builds on the previous one, starting from a blinking LED and working up to timers, interrupts, an RTOS, sensors and industrial communication protocols.

## Projects

| #    | Project                                                | Topics                                                  |
| ---- | ------------------------------------------------------ | ------------------------------------------------------- |
| 01.1 | [Blinky: HAL loop](01_blinky-evolution/01_hal-loop)    | GPIO input/output, polling, HAL, Makefile build         |
| 01.2 | [Blinky: bare-metal](01_blinky-evolution/02_baremetal) | Direct register access, GPIO, startup code              |
| 01.3 | Blinky: timer interrupts                               | Hardware timers, interrupts (NVIC), non-blocking code   |
| 01.4 | Blinky: FreeRTOS tasks                                 | RTOS tasks, scheduling, delays without blocking the CPU |
| 02   | Weather station                                        | Sensors over I2C/SPI, data processing, output           |
| 03   | Modbus slave                                           | UART, the Modbus RTU protocol, register map             |

## License

See [LICENSE.md](LICENSE.md) for licensing terms and third-party code details.
