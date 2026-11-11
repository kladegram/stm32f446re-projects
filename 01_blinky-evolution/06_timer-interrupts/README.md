# Blinky: timer interrupts

Button-controlled LED blinking for the NUCLEO-F446RE using TIM2
and external interrupts.

TIM2 toggles the green LED (PA5) every 1000 ms, or every 100 ms while
the blue user button (PC13) is held. Each toggle sends `LED ON` or `LED OFF`
through USART2. Reusable peripheral modules are kept in `drivers/`.

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

Previous: [FreeRTOS tasks](../05_freertos-tasks/).

## License

See [LICENSE.md](../../LICENSE.md) for original and third-party code licenses.
