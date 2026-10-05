# Blinky: HAL loop

The first step of the blinky series: a single polling `while (1)` loop using STM32 HAL, with blocking delays and no RTOS. The next step, [bare-metal](../02_baremetal), will use direct register access.

The green user LED blinks slowly (it toggles every 500 ms). While the blue user button is held down, it blinks fast (it toggles every 50 ms).

## How it works

| Part        | Board label       | Chip pin | In code                |
| ----------- | ----------------- | -------- | ---------------------- |
| Green LED   | LD2 (Arduino D13) | PA5      | `GPIOA`, `GPIO_PIN_5`  |
| Blue button | B1 USER           | PC13     | `GPIOC`, `GPIO_PIN_13` |

Each pass through the loop in [main.c](Core/Src/main.c):

1. Reads the button with `HAL_GPIO_ReadPin`. The button is active-low: it connects PC13 to ground, so it reads `GPIO_PIN_RESET` while pressed.
2. Toggles the LED with `HAL_GPIO_TogglePin`.
3. Waits 50 ms (button pressed) or 500 ms (released) with `HAL_Delay`.

The button is checked only once per pass, so a press can take up to 500 ms to show.

## Usage

**Requirements:** a NUCLEO-F446RE board connected over USB, and the `arm-none-eabi-gcc`, `make` and `st-flash` tools (in Linux or WSL).

From this folder:

```sh
make          # build to build/01_hal-loop.elf, .hex and .bin
make flash    # build, then write the .bin to the board
make clean    # delete build/
```

## License

See [LICENSE.md](../../LICENSE.md) for licensing terms and third-party code details.
