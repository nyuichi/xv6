#ifndef __ASSEMBLER__
#include "memlayout.h"
#include "traps.h"
#endif

#define PL_USER 3
#define PL_KERN 0

#define HEADER_SIZE 4

#ifndef __ASSEMBLER__

// Routines to let C code use special GAIA instructions and memory addresses.

static inline void
cli(void)
{
  *(int*)INTENABLE = 0;
}

static inline void
sti(void)
{
  *(int*)INTENABLE = 1;
}

// read interrupt flag
static inline uchar
is_interruptible(void)
{
  return *(int*)INTENABLE;
}


static inline void
setpd(uint val)
{
  *(int*)PDADDR = val;
  __asm(".space 40\n"); // 10 nops
}

// read trap no
static inline uint
readtrapno()
{
  return *(int*)CAUSE;
}
// read trap return address
static inline uint
readtreturn()
{
  switch(readtrapno()){
    case T_SYSCALL:
      return *(int*)EPC; // GAIA stores the correct interrupted address for sysenter exception.
    default:
      return *(int*)EPC - 4; // GAIA stores interrupted address + 4 for the others.
  }
}

// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().
struct trapframe {
  uint privilege;
  // trapno: set in trap.c
  uint trapno;
  uint retaddr;

  // general registers
  uint r31;
  uint r30;
  uint r29;
  uint r28;
  uint r27;
  uint r26;
  uint r25;
  uint r24;
  uint r23;
  uint r22;
  uint r21;
  uint r20;
  uint r19;
  uint r18;
  uint r17;
  uint r16;
  uint r15;
  uint r14;
  uint r13;
  uint r12;
  uint r11;
  uint r10;
  uint r9;
  uint r8;
  uint r7;
  uint r6;
  uint r5;
  uint r4;
  uint r3;
  uint r2;
  uint r1;
};
#endif
