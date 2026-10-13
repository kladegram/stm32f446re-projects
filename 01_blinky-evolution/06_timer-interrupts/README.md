# Blinky: timer interrupts

Sixth step of the blinky series, following [FreeRTOS tasks](../05_freertos-tasks).

TIM2 toggles the green LED on PA5 every 1000 ms, or every 100 ms while the blue
B1 button is held. EXTI13 detects presses and releases on PC13. Each toggle
logs `LED ON` or `LED OFF` through USART2 at **115200 baud, 8N1**.

The firmware uses the default 16 MHz clock and reusable LED, UART, button and
timer modules in `drivers/`. Their headers describe the APIs and requirements.

## Build and flash

Requirements: `arm-none-eabi-gcc`, `make`, and `st-flash`.

```sh
make          # build firmware.elf, firmware.bin, and firmware.hex in build/
make flash    # build if needed, flash the binary, and reset the board
make clean    # delete build/
```

## License

See [LICENSE.md](../../LICENSE.md) for licensing terms and third-party code details.
