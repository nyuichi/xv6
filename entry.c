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
  entrypgtable[0] |= 0x80000000;
  entrypgtable[1] |= 0x80000000;
  // Map VA's [0, EXTMEM) to PA's [KERNBASE, KERNLINK)
  // Map VA's [EXTMEM, 512KB) to PA's [EXTMEM, 512KB)
  entrypgdir[0] = (pde_t) entrypgtable | PTE_P | PTE_W;
  // Map VA's [KERNLINK, KERNBASE+512) to PA's [EXTMEM,512)
  // Map VA's [KERNBASE, KERNLINK) to PA's [KERNBASE, KERNLINK]
  entrypgdir[KERNBASE >> PDXSHIFT] = (pde_t) entrypgtable | PTE_P | PTE_W;
} 
