# === Настройки ===
MCU = atmega32
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy

SRC_DIR := ./  # или укажи ./src, если всё в папке
TARGET = main

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall \
         -I./3D_Control -I./3D_Printer -I./Libs -I./PIDR

LDFLAGS = -mmcu=$(MCU)

SRC := $(shell find $(SRC_DIR) -name '*.c')
OBJ := $(SRC:.c=.o)
HEX := $(TARGET).hex

# === Основное правило ===
all: clean $(HEX) flash

# === Компиляция ELF ===
$(TARGET).elf: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# === Генерация HEX ===
$(HEX): $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# === Компиляция C → object ===
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# === Очистка ===
clean:
	@echo Cleaning...
	@rm -f $(TARGET).elf $(OBJ) $(HEX)

# === Прошивка ===
flash: $(HEX)
	avrdude -c usbasp -p $(MCU) -U flash:w:$(HEX):i -B 20

.PHONY: all clean flash
