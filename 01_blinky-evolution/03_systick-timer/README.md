# Blinky: SysTick timer

Third step of the blinky series, following the [bare-metal example](../02_baremetal).

Register-level LED blinking using the Cortex-M4's SysTick timer. A SysTick
exception increments a millisecond counter every 1 ms, and the main loop uses
that counter to toggle the LED on PA5 every 500 ms without a blocking delay.

The timer uses the default 16 MHz HSI clock after reset, with a reload value of
`16000 - 1`. Unsigned subtraction keeps the elapsed-time check working when the
millisecond counter wraps around.

## Build and flash

Requirements: `arm-none-eabi-gcc`, `make`, and `st-flash`.

```sh
make          # build firmware.elf, firmware.bin, and firmware.hex in build/
make flash    # build if needed, flash the binary, and reset the board
make clean    # delete build/
```

## License

See [LICENSE.md](../../LICENSE.md) for licensing terms and third-party code details.
