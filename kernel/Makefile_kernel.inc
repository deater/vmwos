CC = gcc
AS = as
AR = ar
RANLIB = ranlib
ASFLAGS = -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
#ASFLAGS_ARM1176 = -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
ASFLAGS_ARM1176 = -mcpu=arm1176jzf-s -mfloat-abi=hard
#ASFLAGS_ARMV7 = -mcpu=cortex-a7 -mfloat-abi=hard
ASFLAGS_ARMV7 = -mcpu=cortex-a7
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -nostartfiles -ffreestanding -nostdlib
#CFLAGS_ARMV7 = -O2 -Wall -Werror -DARMV7=1 -mcpu=cortex-a7 -mfloat-abi=hard -marm -nostartfiles -ffreestanding -nostdlib
CFLAGS_ARMV7 = -O2 -Wall -DARMV7=1 -mcpu=cortex-a7 -marm -nostartfiles -ffreestanding -nostdlib
#CFLAGS_ARM1176 = -O2 -Wall -Werror -DARM1176=1 -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -nostartfiles -ffreestanding -nostdlib -mgeneral-regs-only
CFLAGS_ARM1176 = -O2 -Wall -DARM1176=1 -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib

LFLAGS = --no-undefined -T $(LINKER_SCRIPT)
