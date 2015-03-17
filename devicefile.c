// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <xv6/param.h>
#include <xv6/fs.h>
#include <termios.h>
#include "defs.h"
#include "traps.h"
#include "spinlock.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"

extern char _binary__min_rt_start[];
extern char _binary__min_rt_size[];

int
minrtread(struct inode *ip, char *dst, int n)
{

  static unsigned offset = 0;
  unsigned sz = *(int*)(_binary__min_rt_start);
  int i;

  for(i=0; i<n; ++i) {
    dst[i] = _binary__min_rt_start[offset];
    offset = (offset + 1) % ((unsigned)_binary__min_rt_size);
  }

  return n;
}


void
minrtinit(void)
{

  devsw[MINRT].write = 0;
  devsw[MINRT].read  = minrtread;
  devsw[MINRT].ioctl = 0;

}

void
deviceinit(void)
{
  minrtinit();
}
