#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "gaia.h"

.global alltraps
alltraps:
  # Check whether interrupt happens in kernel mode or user mode
  mov  k0, cpu
  mov  k0, [k0]
  mov  k0, [k0+0]   # k0 = cpu->privilege
  mov  k1, PL_KERN
  cmpeq k1, k0, k1
  mov   k0, rsp      # store rsp in k0 to make conditions meet with (1)
  bnz    k1, alltraps_trapframe # if we are in kernel mode then jump
alltraps_stackchange:
  # stack: User -> Kernel
  mov  k0, proc
  mov  k0, [k0]
  mov  k0, [k0+8]
  mov  k1, KSTACKSIZE
  add  k1, k0, k1
  mov  k0, rsp       # save previous stack pointer in k0 -- (1)
  mov  rsp, k1       # load kernel stack

alltraps_trapframe:
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
  push k0           # previous stack is stored in k0
  push r31

  # Set in trap
  push k0  # dummy for return addr
  push k0  # dummy for trapno

  # push cpu->privilege
  mov r1, cpu
  mov r1, [r1]
  mov r2, [r1+0]
  push r2
  # cpu->privilege = PL_KERN
  mov [r1+0], PL_KERN

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

  # cpu->privilege = tf->privilege
  mov r1, cpu
  mov r1, [r1]   
  mov r2, [rsp+0]
  mov [r1+0], r2

  # EPC = tf->retaddr
  mov r1, [rsp+8]
  mov r2, EPC
  mov [r2], r1

  mov r31, [rsp+12]
  mov r29, [rsp+20]
  mov r28, [rsp+24]
  mov r27, [rsp+28]
  mov r26, [rsp+32]
  mov r25, [rsp+36]
  mov r24, [rsp+40]
  mov r23, [rsp+44]
  mov r22, [rsp+48]
  mov r21, [rsp+52]
  mov r20, [rsp+56]
  mov r19, [rsp+60]
  mov r18, [rsp+64]
  mov r17, [rsp+68]
  mov r16, [rsp+72]
  mov r15, [rsp+76]
  mov r14, [rsp+80]
  mov r13, [rsp+84]
  mov r12, [rsp+88]
  mov r11, [rsp+92]
  mov r10, [rsp+96]
  mov r9,  [rsp+100]
  mov r8,  [rsp+104]
  mov r7,  [rsp+108]
  mov r6,  [rsp+112]
  mov r5,  [rsp+116]
  mov r4,  [rsp+120]
  mov r3,  [rsp+124]
  mov r2,  [rsp+128]
  mov r1,  [rsp+132]
  # stack: Kernel -> User
  mov r30, [rsp+16]
  break 4

  sysexit
