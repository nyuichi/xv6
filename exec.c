#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "gaia.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  int program_size;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  cprintf("exec called\n");
  begin_op();
  cprintf("exec, begin_op end\n");
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  cprintf("exec, ilock start\n");
  ilock(ip);
  pgdir = 0;

  cprintf("exec, check header\n");
  // Check ELF header
  if(readi(ip, (char*)&program_size, 0, sizeof(int)) < sizeof(int))
    goto bad;
  cprintf("exec, program size: 0x%x\n", program_size);

  cprintf("exec, setupkvm\n");
  if((pgdir = setupkvm()) == 0)
    goto bad;

  cprintf("exec, load program into memory\n");
  // Load program into memory.
  off = HEADER_SIZE;
  if((sz = allocuvm(pgdir, 0, program_size)) == 0)
    goto bad;
  if(loaduvm(pgdir, (char*)0, ip, HEADER_SIZE, program_size) < 0)
    goto bad;

  iunlockput(ip);
  end_op();
  ip = 0;

  cprintf("exec, allocate pages\n");
  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  cprintf("exec, push argument\n");
  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake rbp
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  cprintf("exec, save program name\n");
  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  cprintf("exec, commit to the user image\n");
  // Commit to the user image.
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz;
  proc->tf->retaddr = 0;  // entry point of user programs
  proc->tf->r30 = sp;
  switchuvm(proc);
  freevm(oldpgdir);
  cprintf("exec exited normally\n");
  return 0;

 bad:
  cprintf("exec bad\n");
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
