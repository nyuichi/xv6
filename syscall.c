#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "gaia.h"
#include "syscall.h"

// User code makes a system call with syscall
// System call number in r1.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user rsp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;

  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)proc->sz;
  for(s = *pp; s < ep; s++)
    if(*s == 0)
      return s - *pp;
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint(proc->tf->r30 + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;

  if(argint(n, &i) < 0)
    return -1;
  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

int callsys (int num) {
  switch(num){
  case SYS_fork   : return sys_fork();
  case SYS_exit   : return sys_exit();
  case SYS_wait   : return sys_wait();
  case SYS_pipe   : return sys_pipe();
  case SYS_read   : return sys_read();
  case SYS_kill   : return sys_kill();
  case SYS_exec   : return sys_exec();
  case SYS_fstat  : return sys_fstat();
  case SYS_chdir  : return sys_chdir();
  case SYS_dup    : return sys_dup();
  case SYS_getpid : return sys_getpid();
  case SYS_sbrk   : return sys_sbrk();
  case SYS_sleep  : return sys_sleep();
  case SYS_uptime : return sys_uptime();
  case SYS_open   : return sys_open();
  case SYS_write  : return sys_write();
  case SYS_mknod  : return sys_mknod();
  case SYS_unlink : return sys_unlink();
  case SYS_link   : return sys_link();
  case SYS_mkdir  : return sys_mkdir();
  case SYS_close  : return sys_close();
  default         : return -1;
  }
}

void
syscall(void)
{
  int num;
  num = proc->tf->r1;
  if(num <= 0){
    cprintf("%d %s: unknown sys call %d\n", proc->pid, proc->name, num);
    panic("unknown sys call");
  } else {
    proc->tf->r1 = callsys(num);
  }
}
