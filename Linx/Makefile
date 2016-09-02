
######################
# Makefile for Linx #
######################


# Entry point of Linx
# It must be as same as 'KernelEntryPointPhyAddr' in load.inc!!!
ENTRYPOINT	= 0x30400

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400


# Programs, flags, etc.
ASM		= nasm
DASM		= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -I include/ -c -fno-builtin -fno-stack-protector -m32
LDFLAGS		= -s -Ttext $(ENTRYPOINT) -m elf_i386
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
LINXBOOT	= boot/boot.bin boot/loader.bin
LINXKERNEL	= kernel.bin
OBJS		= kernel/kernel.o kernel/syscall.o kernel/start.o kernel/main.o\
			kernel/clock.o kernel/i8259.o kernel/global.o kernel/protect.o\
			kernel/proc.o kernel/keyboard.o kernel/tty.o kernel/console.o\
			kernel/printf.o kernel/vsprintf.o\
			lib/klib.o lib/klibc.o lib/string.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
everything : $(LINXBOOT) $(LINXKERNEL)

all : realclean everything

final : all clean

image : final buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(LINXBOOT) $(LINXKERNEL)

disasm :
	$(DASM) $(DASMFLAGS) $(LINXKERNEL) > $(DASMOUTPUT)

# Write "boot.bin" & "loader.bin" into floppy image "LINX.IMG"
# We assume that "LINX.IMG" exists in current folder
buildimg :
	#dd if=boot/boot.bin of=LINX.IMG bs=512 count=1 conv=notrunc	
	sudo mount LINX.IMG /tmp/Linx/ -o loop
	sudo cp -f boot/loader.bin /tmp/Linx/
	sudo cp -f kernel.bin /tmp/Linx/
	sudo umount  /tmp/Linx/

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(LINXKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(LINXKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/syscall.o : kernel/syscall.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o: kernel/start.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/main.o: kernel/main.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c include/type.h include/const.h include/protect.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c include/type.h include/const.h include/protect.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/keymap.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c include/type.h include/const.h include/protect.h include/string.h include/proc.h \
			include/tty.h include/console.h include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/vsprintf.o: kernel/vsprintf.c include/type.h include/const.h include/string.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c include/type.h include/const.h
	$(CC) $(CFLAGS) -o $@ $<

lib/klibc.o: lib/klib.c include/type.h include/const.h include/protect.h include/string.h include/proc.h include/proto.h \
			include/tty.h include/console.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

lib/klib.o : lib/klib.asm include/sconst.inc
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<
