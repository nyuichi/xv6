#include "mmu.h"
#include "memlayout.h"

.global alltraps
alltraps:
  # stack: User -> Kernel
  push rsp
  push r1
  push r2
  mov  r1, kpgdir
  mov  r1, [r1]
  mov  r2, KERNBASE
  sub  r1, r1, r2
  mov  r2, P2V_WO(PDEADDR)
  mov  r2, [r2]
  cmpeq r1, r1, r2
  bnz   r1, alltraps_trapframe
alltraps_stackchange:
  mov  r1, proc
  mov  r1, [r1+8]
  mov  rsp, r1
alltraps_trapframe:
  pop  r1
  pop  r2
  # Build trap frame.
  push r1
  push r2
  push r3
  push r4
  push r5
  push r6
  push r7
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
  push r16
  push r17
  push r18
  push r19
  push r20
  push r21
  push r22
  push r23
  push r24
  push r25
  push r26
  push r27
  push r28
  push r29
  push r30
  push r31

  # Set in trap
  push r1  # dummy for return addr
  push r1  # dummy for trapno

  # Call trap(tf), where tf=%rsp
  mov r1, rsp
  sub rsp, rsp, 4
  mov [rsp], r1
  mov  r1, trap
  call r1
  add rsp, rsp, 4

  # Return falls through to trapret...
.global trapret
trapret:
  pop r1   # trapno
  pop r28  # return pop
  add r28, r28, 4 # ok?

  pop r31 
  pop r30 
  pop r29 
          # pop   r28 
  pop r27 # dummy pop
  pop r27 
  pop r26 
  pop r25 
  pop r24 
  pop r23 
  pop r22 
  pop r21 
  pop r20 
  pop r19 
  pop r18 
  pop r17 
  pop r16 
  pop r15 
  pop r14 
  pop r13 
  pop r12 
  pop r11 
  pop r10 
  pop r9 
  pop r8 
  pop r7 
  pop r6 
  pop r5 
  pop r4 
  pop r3 
  pop r2 
  pop r1

  # stack: Kernel -> User
  pop rsp
  
  ret
