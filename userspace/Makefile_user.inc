CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -DVMWOS=1 -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC -Iinclude -Ivlibc/include -I../vlibc/include
OBJCOPYFLAGS = -R .note -R .comment -R .note.gnu.build-id
LFLAGS = -Tuser.ld

ELF2BFLT = ../elf2bflt/elf2bflt
