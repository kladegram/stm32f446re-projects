# Blinky: HAL loop

A basic LED and button example for the NUCLEO-F446RE using STM32 HAL
and blocking delays.

The green LED (PA5) toggles every 500 ms, or every 50 ms while the blue
user button (PC13) is held. The button is checked once per loop.

## Build and flash

Requires `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
Connect the board through its ST-LINK USB port and run from this directory:

```sh
make
make flash
make clean
```

Build output is saved in `build/`.

Next: [Bare-metal](../02_baremetal/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
