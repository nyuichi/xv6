# Initial process execs /init.

#include <sys/syscall.h>
#include "traps.h"
#include "memlayout.h"


# exec(init, argv)
.global _start
_start:
  br start
# char init[] = "/init\0";

init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.align 4
argv:
  .int init
  .int 0

start:
  add rsp, rsp, -12
  mov [rsp+8], argv
  mov [rsp+4], init
  mov [rsp], 0  //dummy rbp
  mov r1, SYS_exec
  sysenter

# for(;;) exit();
exit:
  mov r1, SYS_exit
  sysenter
  br exit
