#include <sys/types.h>
#include "gaia.h"
#include "memlayout.h"

extern uint entrypgdir[];

int
sys_halt(void)
{
  cli(); // interrupt off

  // set pde
  *(int*)PDADDR = V2P(entrypgdir);
  __asm(".space 40\n");

  // go to phys addr
  __asm("mov r1, sys_halt_destination - 0x80000000\n");
  __asm("jr r1\n");
  __asm("sys_halt_destination:\n");

  *(int*)VAENABLE = 0; // mmu off

  // go to boot loader
  __asm("mov r1, 0x80000000\n");
  __asm("jr r1\n");
}
