# Context switch
#
#   void swtch(struct context **old, struct context *new);
#
# Save current register context in old
# and then load register context from new.

#include "memlayout.h"

.global swtch
swtch:
  # Stash args
  mov r1, [rbp+4]
  mov r2, [rbp+8]

  # Save old callee-save registers
  push  rbp
  push  r29
  push  r28

  # Switch stacks
  mov [r1+0], rsp
  mov rsp, r2

  # Load new callee-save registers
  pop r28
  pop r29
  pop rbp

  ret
