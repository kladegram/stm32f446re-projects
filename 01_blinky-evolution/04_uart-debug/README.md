# Blinky: UART debug

Fourth step of the blinky series, following the [SysTick example](../03_systick-timer).

The main loop toggles the green LED on PA5 and sends a repeating counter
from `0` to `9` through USART2. Each digit appears on its own line.
PA2 uses alternate function 7 for USART2 transmit.

The firmware uses CMSIS device headers and the default 16 MHz clock after reset.
A busy-wait of 1,000,000 loop iterations provides an approximate delay;
the counter represents loop iterations, not elapsed time.

## Build and flash

Requirements: `arm-none-eabi-gcc`, `make`, and `st-flash`.

```sh
make          # build firmware.elf, firmware.bin, and firmware.hex in build/
make flash    # build if needed, flash the binary, and reset the board
make clean    # delete build/
```

Connect the ST-LINK USB port and open its virtual COM port at **115200 baud,
8 data bits, no parity, 1 stop bit, no flow control**. On Linux or WSL:

```sh
picocom -b 115200 -d 8 -y n -p 1 -f n /dev/ttyACM0
```

Use your actual serial device name.

## License

See [LICENSE.md](../../LICENSE.md) for licensing terms and third-party code details.
