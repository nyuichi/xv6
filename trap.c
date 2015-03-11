#include <sys/types.h>
#include <xv6/param.h>
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"
#include "traps.h"
#include "spinlock.h"

struct spinlock tickslock;
uint ticks;

extern void alltraps();

void
trapinit(void)
{
  *(int*)INTHANDLER = (int)alltraps;
  initlock(&tickslock, "time");
}

void
trap(struct trapframe *tf)
{
  tf->trapno = readtrapno();
  tf->retaddr = readtreturn();

  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    break;
  case T_COM1:
    uartintr();
    break;

  default:
    if(proc == 0 || cpu->privilege == PL_KERN){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d pc 0x%x\n",
              tf->trapno, cpu->id, tf->retaddr);
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d pc 0x%x --kill proc\n",
        proc->pid, proc->name, tf->trapno, tf->retaddr);
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed)
    exit();
}
