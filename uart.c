// Intel 8250 serial port (UART).

#include <sys/types.h>
#include <sys/file.h>
#include <xv6/param.h>
#include <xv6/fs.h>
#include "defs.h"
#include "traps.h"
#include "spinlock.h"
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
microdelay()
{
}

void
uartputc(int c)
{
  int i;
  if(!uart)
    return;
  while(*(int*)SERIALWE == 0) microdelay();
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
