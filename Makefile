ASMS = \
	_bio.s\
	_console.s\
	_exec.s\
	_file.s\
	_fs.s\
	_ide.s\
	_kalloc.s\
	_log.s\
	_main.s\
	_mp.s\
	_picirq.s\
	_pipe.s\
	_proc.s\
	_spinlock.s\
	_string.s\
	_swtch.s\
	_syscall.s\
	_sysfile.s\
	_sysproc.s\
	_timer.s\
	_trapasm.s\
	_trap.s\
	_uart.s\
	_vm.s\

NATIVECC = clang
UCCDIR = ../ucc
UCCLIBS = $(UCCDIR)/lib/libucc.s

CC = $(UCCDIR)/bin/ucc
AS = $(UCCDIR)/bin/as
SIM = $(UCCDIR)/bin/sim
CFLAGS = -I.
ASFLAGS = -s
VPATH = lib:usr

xv6.img: kernelmemfs
	dd if=kernelmemfs of=xv6.img conv=notrunc

xv6nomemfs.img: kernel fs.img
	dd if=kernel of=xv6nomemfs.img conv=notrunc

bootblock: bootasm.S bootmain.c
	# TODO: do nothing now

initcode: initcode.S
	$(NATIVECC) -E -o _initcode.s $<
	$(AS) _initcode.s -r -e 0 -o initcode

# TODO: Concatenate initcode. Make the entry point proper one.
kernel: $(ASMS) initcode
	$(AS) $(ASFLAGS) -o kernel $(ASMS) $(UCCLIBS) -f __UCC_HEAP_START

# kernelmemfs is a copy of kernel that maintains the
# disk image in memory instead of writing to a disk.
# This is not so useful for testing persistent storage or
# exploring disk buffering implementations, but it is
# great for testing the kernel on real hardware without
# needing a scratch disk.
# We use memfs as default because our CPU architecture has no disk.
# TODO: Concatenate initcode and fs.img. Make the entry point proper one.
MEMFSASMS = $(filter-out _ide.s,$(ASMS)) _memide.s
kernelmemfs: $(MEMFSASMS) initcode fs.img
	$(AS) $(ASFLAGS) -o kernelmemfs $(MEMFSASMS) $(UCCLIBS) -f __UCC_HEAP_START

tags: $(ASMS) _init
	etags *.S *.c

ULIB = lib/_ulib.s lib/_usys.s lib/_printf.s lib/_umalloc.s

_%.s: %.S
	$(NATIVECC) -E -I. -o $@ $<

_%.s: %.c
	$(CC) $(CFLAGS) -s -o $@ $<

lib/_usys.s: lib/usys.S
	$(NATIVECC) -E -I. -o $@ $<
	sed -i "s/;/\n/g" $@

_%: _%.s $(ULIB)
	$(AS) -s -e 0 -o $@ $^ $(UCCLIBS) -f __UCC_HEAP_START

_forktest: usr/_forktest.s $(ULIB)
	$(AS) -s -e 0 -o $@ usr/_forktest.s lib/_ulib.s lib/_usys.s $(UCCLIBS) -f __UCC_HEAP_START

mkfs: tools/mkfs.c fs.h
	gcc -Werror -Wall -o mkfs tools/mkfs.c -I.. -idirafter . -g

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	_cat\
	_echo\
	_forktest\
	_grep\
	_init\
	_kill\
	_ln\
	_ls\
	_mkdir\
	_rm\
	_sh\
	_stressfs\
	_wc\
	_zombie\
	#_usertests\ # remove this to save file system size.

fs.img: mkfs README $(UPROGS)
	./mkfs fs.img README $(UPROGS)

-include *.d

clean:
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*.o *.d *.asm *.sym vectors.S bootblock \
	initcode initcode.out kernel xv6.img fs.img kernelmemfs mkfs \
	.gdbinit \
	$(ASMS) \
	$(UPROGS) _*.s \
	lib/*.o lib/*.d lib/_*.s \
	usr/*.o usr/*.d usr/*.sym usr/*.asm usr/_*.s

sim: xv6.img
	$(SIM) -no-interrupt xv6.img
