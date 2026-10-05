TARGET = firmware
BUILD_DIR = build

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size
ST_FLASH = st-flash

MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS = $(MCU_FLAGS) -std=c11 -Og -g3 -Wall -Wextra \
         -ffreestanding -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS = $(MCU_FLAGS) -g3 -x assembler-with-cpp -MMD -MP
LDFLAGS = $(MCU_FLAGS) -nostdlib -Tlinker.ld \
          -Wl,--gc-sections,-Map=$(BUILD_DIR)/$(TARGET).map
LDLIBS = -lgcc

OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/startup.o

.PHONY: all flash clean
.DELETE_ON_ERROR:

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/main.o: main.c Makefile | $(BUILD_DIR)
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
	$(ST_FLASH) --reset write $< 0x08000000

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
