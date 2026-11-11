# Blinky: SysTick timer

LED timing for the NUCLEO-F446RE using the Cortex-M4's SysTick timer
and direct register access.

A SysTick interrupt updates a millisecond counter. The main loop uses it
to toggle the green LED (PA5) every 500 ms without a blocking delay.
The timer uses the default 16 MHz clock.

## Build and flash

Requires `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
Connect the board through its ST-LINK USB port and run from this directory:

```sh
make
make flash
make clean
```

Build output is saved in `build/`.

Previous: [Bare-metal](../02_baremetal/). Next: [UART debug](../04_uart-debug/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
