#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"

//static void startothers(void);
static void mpmain(void);
extern pde_t *kpgdir;

extern char __UCC_HEAP_START;
char *end;
//extern char end[]; // first address after kernel loaded from ELF file

void init_global_var();

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
  init_global_var();

  // virtual addr is not yet implemented?
  //kinit1(end, end + 4*1024*70/4); // phys page allocator
  kvmalloc();      // kernel page table
  mpinit();       // collect info about this machine
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);

  consoleinit();   // I/O devices & their interrupts
  uartinit();      // serial port

  cprintf("pinit...\n");
  pinit();         // process table
  trapinit();      // trap vectors
  binit();         // buffer cache
  cprintf("fileinit...\n");
  fileinit();      // file table
  cprintf("iinit...\n");
  iinit();         // inode cache
  //ideinit();     // @memide.c

  kinit2(end + 4*1024*70/4, P2V(PHYSTOP)); // must come after startothers()

  cprintf("userinit...\n");
  //userinit();      // first user process
  cprintf("mpmain...\n");
  // Finish setting up this processor in mpmain.
  mpmain();
}


/*
// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}
*/
// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting\n", cpu->id);
  idtinit();       // load idt register
  cpu->started = 1;
  scheduler();     // start running processes
}

// Boot page table used in entry.S and entryother.S.
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.
// Use PTE_PS in page directory entry to enable 4Mbyte pages.

pde_t entrypgdir[NPDENTRIES];

void init_global_var() {
  end = (char*)&__UCC_HEAP_START;
  return;
}

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
