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
  //kinit1(end, P2V(4*1024*70)); // phys page allocator
  kvmalloc();      // kernel page table
  seginit();       // set up segments
  cprintf("\ncpu%d: starting xv6\n\n", cpu->id);

  consoleinit();   // I/O devices & their interrupts
  uartinit();      // serial port

  pinit();         // process table
  trapinit();        // trap vectors
  binit();         // buffer cache
  fileinit();      // file table
  iinit();         // inode cache
  ideinit();       // disk
  if(!ismp)
    timerinit();   // uniprocessor timer

  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()

  userinit();      // first user process
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
  xchg(&cpu->started, 1); // tell startothers() we're up
  scheduler();     // start running processes
}

// Boot page table used in entry.S and entryother.S.
// Page directories (and page tables), must start on a page boundary,
// hence the "__aligned__" attribute.
// Use PTE_PS in page directory entry to enable 4Mbyte pages.

pde_t entrypgdir[NPDENTRIES];

void init_global_var() {
  end = P2V(&__UCC_HEAP_START);
  return;
}

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
