
SHELL = bash

# Project Name
PROJECT = copter_arm
# Target MCU (must match a device macro name in "stm32f7xx.h")
MCU_DEFINE = STM32F767xx

OBJDIR = obj
SRCDIR = src
OUTDIR = out

LINKER_SCRIPT = linker/stm32f767ZI.ld

CFLAGS = -D $(MCU_DEFINE)
CCPREFIX = arm-none-eabi-
CC   = $(CCPREFIX)gcc
CP   = $(CCPREFIX)objcopy
AS   = $(CCPREFIX)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary -S
GDB  = $(CCPREFIX)gdb

INCDIRS  = ./src \
           ./src/CMSIS/Include \
           ./src/CMSIS/ST \
           ./src/FreeRTOS/Source/portable/GCC/ARM_CM4F \
           ./src/FreeRTOS/Source/portable/MemMang \
           ./src/FreeRTOS/Source/include
INCDIRS  := $(patsubst %,-I%, $(INCDIRS))

OUT_NAME = $(OUTDIR)/$(PROJECT)

# Sources
SRC := $(wildcard src/*.c) \
       src/CMSIS/ST/system_stm32f7xx.c \
       $(wildcard src/FreeRTOS/Source/*.c) \
       $(wildcard src/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c) \
       $(wildcard src/FreeRTOS/Source/portable/MemMang/*.c)

# Objects
OBJ = $(SRC:%.c=%.o) src/CMSIS/ST/startup_stm32f767xx.o
LST = $(OBJ:%.o=%.lst)

LIB_DIR = /usr/lib/arm-none-eabi/lib/thumb/v7e-m+fp/hard

# Flags
MCU  = cortex-m7
MCFLAGS = -mcpu=$(MCU) -mfloat-abi=hard
DEBUG_FLAGS = -gdwarf-2
ASFLAGS = $(MCFLAGS) $(DEBUG_FLAGS) -mthumb  -Wa,-amhls=$(<:%.s=%.lst)
CFLAGS = $(MCFLAGS) -D $(MCU_DEFINE) $(DEBUG_FLAGS) -O0 -falign-functions=4 -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:%.c=%.lst) $(DEFS)
LDFLAGS = $(MCFLAGS) $(DEBUG_FLAGS) -mthumb -nostartfiles --specs=nosys.specs --specs=nano.specs -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--no-warn-mismatch,--gc-sections -L$(LIB_DIR)

# Test Objects
TEST_OBJ = $(wildcard test/*.c)


# Rules
all : $(OBJ) $(OUT_NAME).elf $(OUT_NAME).hex $(OUT_NAME).bin
	@echo "All targets up to date."

flash: all
	st-flash write $(OUT_NAME).bin 0x8000000

%.elf : $(OBJ) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) -T$(LINKER_SCRIPT) $(OBJ) -o $@

%.hex: %.elf
	$(HEX) $< $@

%.bin: %.elf
	$(BIN) $< $@

%.o : %.c
	$(CC) -c $(CFLAGS) $(INCDIRS) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

debug : 
	$(GDB) -tui --eval-command="target extended-remote localhost:4242" out/$(PROJECT).elf -ex 'load'

test :
	cd tests && cmake -S . -B build/ && cd build && make && ctest

clean :
	rm -f $(OBJ)
	rm -f $(LST)
	rm -rf $(OUTDIR)/$(PROJECT).{elf,hex,bin,map}
	rm -rf tests/build

# Create output directories if they don't already exist.
$(shell mkdir -p $(OUTDIR))
