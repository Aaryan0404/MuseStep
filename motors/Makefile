# Sample makefile for project
# Builds "myprogram.bin" from myprogram.c (edit PROGRAM to change)
# Additional source file(s) mymodule.c (edit SOURCES to change)
# Link against reference libpi (edit LDLIBS, LDFLAGS to change)

PROGRAM = motors.bin
SOURCES = $(PROGRAM:.bin=.c) naj.c

all: $(PROGRAM)

CFLAGS  = -I$(CS107E)/include -Og -g -std=c99 $$warn $$freestanding
CFLAGS += -mapcs-frame -fno-omit-frame-pointer -mpoke-function-name
LDFLAGS = -nostdlib -T memmap -L. -L$(CS107E)/lib
LDLIBS  = -lpi -lgcc

OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

%.bin: %.elf
	arm-none-eabi-objcopy $< -O binary $@

%.elf: $(OBJECTS) %.o
	@echo arm-none-eabi-gcc $(LDFLAGS) $^ $(LDLIBS) -o $@
	@$(CS107E)/bin/link-filter arm-none-eabi-gcc $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $< -o $@

%.o: %.s
	arm-none-eabi-as $< -o $@

%.list: %.o
	arm-none-eabi-objdump --no-show-raw-insn -d $< > $@

run: $(PROGRAM)
	rpi-run.py -p $<

clean:
	rm -f *.o *.bin *.elf *.list

# this rule will provide better error message when
# a source file cannot be found (missing, misnamed)
$(SOURCES):
	$(error cannot find source file `$@` needed for build)

.PHONY: all clean run
.PRECIOUS: %.elf %.o

# disable built-in rules (they are not used)
.SUFFIXES:

export warn = -Wall -Wpointer-arith -Wwrite-strings -Werror \
              -Wno-error=unused-function -Wno-error=unused-variable \
              -fno-diagnostics-show-option
export freestanding = -ffreestanding -nostdinc \
                      -isystem $(shell arm-none-eabi-gcc -print-file-name=include)

define CS107E_ERROR_MESSAGE
ERROR - CS107E environment variable is not set.

Review instructions for properly configuring your shell.
https://cs107e.github.io/guides/install/userconfig#env

endef

ifndef CS107E
$(error $(CS107E_ERROR_MESSAGE))
endif
