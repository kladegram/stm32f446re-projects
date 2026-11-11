# STM32F446RE Projects

A collection of embedded C projects for the ST NUCLEO-F446RE,
exploring GPIO, timers, interrupts, UART, FreeRTOS, and sensor interfacing.

## Hardware

- ST NUCLEO-F446RE development board
- AHT20 temperature and humidity sensor
- Joy-IT SBC-OLED01.3 1.3" OLED 128x64 display
- Dupont jumper wires

## Projects

### [01 — Blinky evolution](01_blinky-evolution/)

A series of LED examples exploring different approaches to embedded programming:

1. **HAL loop** — GPIO control with STM32 HAL and blocking delays.
2. **Bare-metal** — direct register access.
3. **SysTick timer** — LED timing without blocking delays.
4. **UART debug** — serial output through USART2.
5. **FreeRTOS tasks** — tasks, queues, and serial logging.
6. **Timer interrupts** — timer-driven blinking and button interrupts.

### [02 — Weather station](02_weather-station/)

A temperature and humidity station using an AHT20 sensor and a 128×64
OLED display. Includes register-level peripheral drivers, UART output,
and sleep between readings.

## Build and flash

The projects use `arm-none-eabi-gcc`, `make`, and `st-flash`.

Run from an individual project directory:

    make
    make flash

See each project's README for details.

## License

Original source code is licensed under the MIT License.
Third-party components retain their respective licenses.
See [LICENSE.md](LICENSE.md).
