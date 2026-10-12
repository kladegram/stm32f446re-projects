# STM32F446RE Projects

Embedded C projects on the **ST NUCLEO-F446RE** board (Arm Cortex-M4F, up to 180 MHz, 512 KB flash, 128 KB RAM).

This repository is my embedded-programming learning path. Each project builds on the previous one, starting from a blinking LED and working up to timers, interrupts, an RTOS, sensors and industrial communication protocols.

## Projects

| #    | Project                                                    | Topics                                                  |
| ---- | ---------------------------------------------------------- | ------------------------------------------------------- |
| 01.1 | [Blinky: HAL loop](01_blinky-evolution/01_hal-loop)           | GPIO input/output, polling, HAL, Makefile build          |
| 01.2 | [Blinky: bare-metal](01_blinky-evolution/02_baremetal)        | Direct register access, GPIO, startup code               |
| 01.3 | [Blinky: SysTick timer](01_blinky-evolution/03_systick-timer) | SysTick exceptions, millisecond timebase, non-blocking timing |
| 01.4 | [Blinky: UART debug](01_blinky-evolution/04_uart-debug)       | CMSIS device headers, USART2, polling TX, counter output, busy-wait delay |
| 01.5 | [Blinky: FreeRTOS tasks](01_blinky-evolution/05_freertos-tasks) | RTOS tasks, scheduling, queues, UART logging              |
| 01.6 | [Blinky: timer interrupts](01_blinky-evolution/06_timer-interrupts) | Hardware timers, interrupts (NVIC), non-blocking code |
| 02   | Weather station                                            | Sensors over I2C/SPI, data processing, output             |
| 03   | Modbus slave                                               | UART, the Modbus RTU protocol, register map               |

Steps 01.1 through 01.5 contain firmware. Timer interrupts, the weather
station and the Modbus slave are planned projects.

The UART example uses the chip's default 16 MHz clock. Its README includes
instructions for reading the counter output through ST-LINK on Linux or WSL.

## License

See [LICENSE.md](LICENSE.md) for licensing terms and third-party code details.
