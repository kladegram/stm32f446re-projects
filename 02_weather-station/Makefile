TARGET = firmware
BUILD_DIR = build

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size
ST_FLASH = st-flash

# CMSIS supplies register names and bit masks for the STM32F446.
CMSIS_DIR = ../01_blinky-evolution/01_hal-loop/Drivers/CMSIS
CPPFLAGS = -DSTM32F446xx -Idrivers \
           -I$(CMSIS_DIR)/Include \
           -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include
# Software floating point keeps startup independent of the FPU.
MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS = $(MCU_FLAGS) -std=c11 -Og -g3 -Wall -Wextra \
         -ffreestanding -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS = $(MCU_FLAGS) -g3 -x assembler-with-cpp -MMD -MP
# Keep our startup code and link Newlib Nano with bare-metal system-call stubs.
# Add -Wl,-u,_printf_float when the application needs floating-point printf.
LDFLAGS = $(MCU_FLAGS) -nostartfiles --specs=nano.specs --specs=nosys.specs \
          -Tlinker.ld \
          -Wl,--gc-sections,-z,noexecstack,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -Wl,-u,_printf_float
LDLIBS = -lm

C_SOURCES = main.c   
C_SOURCES += drivers/led.c
C_SOURCES += drivers/button.c
C_SOURCES += drivers/usart.c
C_SOURCES += drivers/timer.c
C_SOURCES += drivers/i2c.c
C_SOURCES += drivers/aht20.c
C_SOURCES += drivers/spi.c
C_SOURCES += drivers/SH1106.c
C_SOURCES += lib/ring_buffer.c
C_SOURCES += lib/Font_8x8.c

OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o)) $(BUILD_DIR)/startup.o

.PHONY: all flash clean
.DELETE_ON_ERROR:

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup.o: startup.s Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) linker.ld Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

flash: $(BUILD_DIR)/$(TARGET).bin
	$(ST_FLASH) --connect-under-reset --reset write $< 0x08000000

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
