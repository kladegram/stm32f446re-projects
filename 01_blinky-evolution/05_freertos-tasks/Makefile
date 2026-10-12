TARGET = firmware
BUILD_DIR = build

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size
ST_FLASH = st-flash

# Reuse the existing Arm/ST CMSIS files; GPIO is configured through registers.
CMSIS_DIR = ../01_hal-loop/Drivers/CMSIS
SYSTEM_SOURCE = ../01_hal-loop/Core/Src/system_stm32f4xx.c
FREERTOS_DIR = FreeRTOS-Kernel
FREERTOS_PORT = $(FREERTOS_DIR)/portable/GCC/ARM_CM4F
CPPFLAGS = -DSTM32F446xx -I. \
           -I$(CMSIS_DIR)/Include \
           -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include \
           -I$(FREERTOS_DIR)/include -I$(FREERTOS_PORT)
# The ARM_CM4F port saves floating-point registers during task switches.
MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS = $(MCU_FLAGS) -std=c11 -Og -g3 -Wall -Wextra \
         -ffreestanding -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS = $(MCU_FLAGS) -g3 -x assembler-with-cpp -MMD -MP
LDFLAGS = $(MCU_FLAGS) -nostdlib -Tlinker.ld \
          -Wl,--gc-sections,-z,noexecstack,-Map=$(BUILD_DIR)/$(TARGET).map
# FreeRTOS uses libc memory functions; its heap does not use libc malloc.
LDLIBS = -Wl,--start-group -lc -lgcc -Wl,--end-group

C_SOURCES = main.c uart.c freertos_hooks.c \
            $(FREERTOS_DIR)/tasks.c $(FREERTOS_DIR)/list.c $(FREERTOS_DIR)/queue.c \
            $(FREERTOS_PORT)/port.c $(FREERTOS_DIR)/portable/MemMang/heap_4.c
OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o)) \
          $(BUILD_DIR)/system_stm32f4xx.o $(BUILD_DIR)/startup.o

.PHONY: all flash clean
.DELETE_ON_ERROR:

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup.o: startup.s Makefile
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/system_stm32f4xx.o: $(SYSTEM_SOURCE) Makefile
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) linker.ld Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

flash: $(BUILD_DIR)/$(TARGET).bin
	$(ST_FLASH) --reset write $< 0x08000000

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
