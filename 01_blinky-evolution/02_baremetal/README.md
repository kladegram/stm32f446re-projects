# Blinky: bare-metal

An LED and button example for the NUCLEO-F446RE using direct register access
and manually defined register addresses.

The green LED (PA5) blinks faster while the blue user button (PC13) is held.
Timing uses a busy-wait loop, so delays are approximate.

## Build and flash

Requires `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
Connect the board through its ST-LINK USB port and run from this directory:

```sh
make
make flash
make clean
```

Build output is saved in `build/`.

Previous: [HAL loop](../01_hal-loop/). Next: [SysTick timer](../03_systick-timer/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
