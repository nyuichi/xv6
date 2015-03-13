ASMS = \
        _entryasm.s\
	_entry.s\
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
	_pipe.s\
	_proc.s\
	_spinlock.s\
	_string.s\
	_swtch.s\
	_syscall.s\
	_sysfile.s\
	_syshalt.s\
	_sysproc.s\
	_timer.s\
	_trapasm.s\
	_trap.s\
	_uart.s\
	_vm.s\

NATIVECC = clang
UCCDIR = ../ucc
GAIASOFTDIR = ../gaia-software
UCCLIBS = $(UCCDIR)/lib/libucc.s

CC = $(UCCDIR)/bin/ucc
ifeq ($(wildcard $(GAIASOFTDIR)),) # if gaia-software does not exist then
  AS = $(UCCDIR)/bin/as
  SIM = $(UCCDIR)/bin/sim
else
  AS = $(GAIASOFTDIR)/asm.py
  SIM = $(GAIASOFTDIR)/sim
endif

CFLAGS = -I. -I./include
NATIVECFLAGS = $(CFLAGS)
ASFLAGS = -v -Wno-unused-label
SIMFLAGS = -stat -debug
VPATH = lib:usr

xv6.img: kernelmemfs
	dd if=kernelmemfs of=xv6.img

initcode: initcode.S
	$(NATIVECC) $(NATIVECFLAGS) -E -o _initcode.s $<
	$(AS) -c -Wno-unused-label _initcode.s -r -e 0 -o initcode

# kernelmemfs is a copy of kernel that maintains the
# disk image in memory instead of writing to a disk.
# This is not so useful for testing persistent storage or
# exploring disk buffering implementations, but it is
# great for testing the kernel on real hardware without
# needing a scratch disk.
# We use memfs as default because our CPU architecture has no disk.
MEMFSASMS = $(filter-out _ide.s,$(ASMS)) _memide.s
kernelmemfs: $(MEMFSASMS) initcode fs.img
	./tools/gen_binary_blobs 0x80002000 initcode fs.img
	$(AS) $(ASFLAGS) -c -o _kernelmemfs -e 0x80002000 -start _start $(MEMFSASMS) _binary_blobs.s $(UCCLIBS) -f __UCC_HEAP_START
	./tools/gen_binary_blobs `ruby -e "print open('_kernelmemfs').size + 0x80002000"` initcode fs.img
	$(AS) $(ASFLAGS) -c -o _kernelmemfs -e 0x80002000 -start _start $(MEMFSASMS) _binary_blobs.s $(UCCLIBS) -f __UCC_HEAP_START
	cat _kernelmemfs initcode fs.img > kernelmemfs
	rm _kernelmemfs
	./tools/attach_boot_header kernelmemfs

tags: $(ASMS) _init
	etags *.S *.c

ULIB = lib/_ulib.s lib/_usys.s lib/_printf.s lib/_umalloc.s
LIBC = lib/_libc.s lib/_usys.s

_%.s: %.S
	$(NATIVECC) $(NATIVECFLAGS) -E -I. -o $@ $<

_%.s: %.c
	$(CC) $(CFLAGS) -s -o $@ $<

lib/_usys.s: lib/usys.S
	$(NATIVECC) $(NATIVECFLAGS) -E -I. -o $@ $<
	sed -i "s/;/\n/g" $@

_%: _%.s $(ULIB)
	$(AS) $(ASFLAGS) -e 0 -o $@ $^ $(UCCLIBS) -f __UCC_HEAP_START

_cat: _cat.s $(LIBC)
	$(AS) $(ASFLAGS) -e 0 -o $@ $^ $(UCCLIBS) -f __UCC_HEAP_START

_forktest: usr/_forktest.s $(ULIB)
	$(AS) $(ASFLAGS) -e 0 -o $@ usr/_forktest.s lib/_ulib.s lib/_usys.s $(UCCLIBS) -f __UCC_HEAP_START

_sl: lib/_curses.s $(ULIB) usr/_sl.s
	$(AS) $(ASFLAGS) -e 0 -o $@ usr/_sl.s lib/_curses.s $(ULIB) $(UCCLIBS) -f __UCC_HEAP_START

# use libc from ucc
_2048: $(LIBC) usr/_2048.s
	$(AS) $(ASFLAGS) -e 0 -o $@ usr/_2048.s lib/_curses.s $(LIBC) $(UCCLIBS) -f __UCC_HEAP_START

mkfs: tools/mkfs.c
	gcc -Werror -Wall -o mkfs tools/mkfs.c -idirafter ./include -idirafter . -g

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: _%.s

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
	_sl\
	_sed\
	_halt\
	_2048\
        _minesweeper\
	_pwd\
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
	$(SIM) $(SIMFLAGS) xv6.img
