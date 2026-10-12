# Blinky: FreeRTOS tasks

Fifth step of the blinky series, following [UART debug](../04_uart-debug).

Three FreeRTOS tasks blink the green LED on PA5 every 500 ms, queue a status
message every 3 seconds, and send logs through USART2 on PA2. Each LED toggle
is logged. Only the logging task writes to UART, so messages stay together.

The firmware uses CMSIS headers and the default 16 MHz clock. It follows
stage 7 of [Artful Bytes' RTOS example](https://github.com/artfulbytes/i_kept_coding_until_an_rtos_was_inevitable/blob/main/main.c).

## Build and flash

Requirements: `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
FreeRTOS is included; CMSIS files are reused from `../01_hal-loop`.

Run from this directory:

```sh
make          # build firmware.elf, firmware.bin, and firmware.hex in build/
make flash    # flash the binary and reset the board
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
