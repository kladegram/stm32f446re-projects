# Blinky: FreeRTOS tasks

An example of FreeRTOS tasks, queues, and serial logging for the NUCLEO-F446RE.

Three tasks toggle the green LED (PA5) every 500 ms, queue a status message
every 3 seconds, and send queued logs through USART2 (PA2).
Only the logging task writes to UART, keeping messages together.

Based on stage 7 of [Artful Bytes' RTOS example](https://github.com/artfulbytes/i_kept_coding_until_an_rtos_was_inevitable/blob/main/main.c).

## Build and flash

Requires `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
Connect the board through its ST-LINK USB port and run from this directory:

```sh
make
make flash
make clean
```

Build output is saved in `build/`. FreeRTOS is included;
CMSIS files are reused from `../01_hal-loop/`.

## Serial output

Open the ST-LINK virtual COM port at **115200 baud, 8N1, no flow control**.
For example, on Linux or WSL:

```sh
screen /dev/ttyACM0 115200
```

Replace `/dev/ttyACM0` with your serial device.

Previous: [UART debug](../04_uart-debug/). Next: [Timer interrupts](../06_timer-interrupts/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
