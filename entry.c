#include "types.h"
#include "mmu.h"
#include "memlayout.h"

void set_entrypgdir(pde_t *entrypgdir, pde_t *entrypgtable)
{
  int i;

  // Create boot page table.
  for (i = 0; i < 128; i++) {
    entrypgtable[i] = (i << 12) | PTE_P | PTE_W;
  } 
  // Map VA's [0, 512KB) to PA's [0, 512KB)
  entrypgdir[0] = (pde_t) entrypgtable | PTE_P | PTE_W;
  // Map VA's [KERNBASE, KERNBASE + 512KB) to PA's [0, 512KB)
  entrypgdir[KERNBASE >> PDXSHIFT] = (pde_t) entrypgtable | PTE_P | PTE_W;
}
