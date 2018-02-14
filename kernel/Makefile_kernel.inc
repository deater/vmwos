CC = gcc
AS = as
AR = ar
RANLIB = ranlib
ASFLAGS = -mcpu=arm1176jzf-s
ASFLAGS_ARMV6 = -mcpu=arm1176jzf-s
ASFLAGS_ARMV7 = -mcpu=cortex-a7
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib
CFLAGS_ARMV7 = -O2 -Wall -mcpu=cortex-a7 -marm -nostartfiles -ffreestanding -nostdlib
CFLAGS_ARMV6 = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib
LFLAGS = --no-undefined -T $(LINKER_SCRIPT)
