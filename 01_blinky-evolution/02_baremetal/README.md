# Blinky: bare-metal

Second step of the blinky series, following the [HAL loop](../01_hal-loop).
Register-level implementation of the LED and button example

## Build and flash

Requirements: `arm-none-eabi-gcc`, `make`, and `st-flash`.

```sh
make          # build firmware.elf, firmware.bin, and firmware.hex in build/
make flash    # build if needed, flash the binary, and reset the board
make clean    # delete build/
```

## License

See [LICENSE.md](../../LICENSE.md) for licensing terms and third-party code details.
