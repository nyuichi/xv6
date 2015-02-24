#include "param.h"
#include "memlayout.h"

.global entry
entry:
  mov rsp, V2P_WO(stack + KSTACKSIZE)

  add rsp, rsp, -8
  mov [rsp - 4], V2P_WO(entrypgtable)
  mov [rsp], V2P_WO(entrypgdir)
  mov r1, V2P_WO(set_entrypgdir)
  call r1

  mov r1, PDADDR
  mov [r1], V2P_WO(entrypgdir)
  mov r1, VAENABLE
  mov [r1], 1
  mov rsp, stack + KSTACKSIZE
  mov r1, main
  jr r1

.global _start
.set _start, V2P_WO(entry)

.align 4096
.global entrypgdir
entrypgdir:
  .int    0, 1024
.global entrypgtable
entrypgtable:
  .int    0, 1024
.global stack
stack:
  .int    0, KSTACKSIZE
