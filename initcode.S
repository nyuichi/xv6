# Initial process execs /init.

#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


# exec(init, argv)
.global _start
_start:
  br start
# char init[] = "/init\0";

init:
  #.string "/init\0"
  .int 0x0000002f
  .int 0x00000069
  .int 0x0000006e
  .int 0x00000069
  .int 0x00000074
  .int 0x00000000

# char *argv[] = { init, 0 };
.align 4
argv:
  .int init
  .int 0

start:
  mov r1, P2V_WO(SERIAL)
  mov [r1], 0x49  // print out 'I' to tell that we are here
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

