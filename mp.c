// GAIA does not support Multiprocessor.
// Only a few initalizations are done here.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "gaia.h"
#include "mmu.h"
#include "proc.h"

struct cpu cpus[NCPU];
struct cpu *cpu;
int ismp;
int ncpu;

void
mpinit(void)
{
  // No MP.
  ncpu = 1;
  cpus[0].id = 0;
  ismp = 0;
; cpu = &cpus[0];
}
