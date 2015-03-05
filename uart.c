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
  if(!uart)
    return;
  *(int*)SERIAL = c;
}

static int
uartgetc(void)
{
  if(!uart)
    return -1;
  return *(int*)SERIAL;
}

void
uartintr(void)
{
  consoleintr(uartgetc);
}
