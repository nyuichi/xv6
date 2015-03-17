#include <sys/syscall.h>
#include "traps.h"

#define SYSCALL(name) \
  .global name; \
  name:; \
    mov r1, SYS_ ## name; \
    sysenter; \
    jr r28

SYSCALL(fork)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(halt)
SYSCALL(ioctl)
SYSCALL(procdump)

.global _exit
_exit:
  mov r1, SYS_exit
  sysenter
  jr r28
