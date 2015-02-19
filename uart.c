// Intel 8250 serial port (UART).

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"
#include "memlayout.h"

#define COM1    0x3f8

static int uart;    // is there a uart?

void
uartinit(void)
{
  char *p;
  uart = 1;
  // Announce that we're here.
  for(p="xv6...\n"; *p; p++){
    uartputc(*p);
  }
}

void
microdelay(int us)
{
}

void
uartputc(int c)
{
  int i;
  /* cprintf is used before uartinit @main.c, but cprintf uses uartputc.
  if(!uart)
    return;
  */
  for(i = 0; i < 128; i++)
    microdelay(10);
  *(int*)P2V(SERIAL) = c;
}

static int
uartgetc(void)
{
  if(!uart)
    return -1;
  if(!(inb(COM1+5) & 0x01))
    return -1;
  return inb(COM1+0);
}

void
uartintr(void)
{
  consoleintr(uartgetc);
}
