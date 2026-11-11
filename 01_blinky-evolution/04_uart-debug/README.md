# Blinky: UART debug

Serial debug output for the NUCLEO-F446RE using USART2 and CMSIS
register definitions.

The main loop toggles the green LED (PA5) and sends a repeating counter
from `0` to `9` over USART2 (PA2), one digit per line.
A busy-wait loop provides an approximate delay; the counter is not a timer.

## Build and flash

Requires `arm-none-eabi-gcc`, `make`, and `st-flash` on Linux or WSL.
Connect the board through its ST-LINK USB port and run from this directory:

```sh
make
make flash
make clean
```

Build output is saved in `build/`. CMSIS headers are reused from `../01_hal-loop/`.

## Serial output

Open the ST-LINK virtual COM port at **115200 baud, 8N1, no flow control**.
For example, on Linux or WSL:

```sh
screen /dev/ttyACM0 115200
```

Replace `/dev/ttyACM0` with your serial device.

Previous: [SysTick timer](../03_systick-timer/). Next: [FreeRTOS tasks](../05_freertos-tasks/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
