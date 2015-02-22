#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
//struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

extern void alltraps();

void
trapinit(void)
{
  *(int*)P2V(INTHANDLER) = (int)alltraps;
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  //lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  tf->trapno = readtrapno();
  tf->retaddr = readtreturn();

  cprintf("trap called. tf->retaddr:0x%x\n", tf->retaddr);
  if(tf->trapno == T_SYSCALL){
    cprintf("syscall interrupt.\n");
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    cprintf("timer interrupt\n");
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    //kbdintr();
    break;
  case T_IRQ0 + IRQ_COM1:
    cprintf("serial interrupt\n");
    uartintr();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    /*
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    */
    cprintf("cpu%d: spurious interrupt\n",
            cpu->id);
    break;

  //PAGEBREAK: 13
  default:
    /*
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    */
    // In user space, assume process misbehaved.
    /*
    cprintf("pid %d %s: trap %d err %d on cpu %d eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip,
            rcr2());
    */
     cprintf("pid %d %s: trap %d --kill proc\n",
            proc->pid, proc->name, tf->trapno);
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed)
    exit();
}
