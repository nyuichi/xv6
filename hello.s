msg:
.string "Hello World!"
.align 4

.global main
main:
ldl r1, msg
loop:
ldb r29, r1, 0x0
add r2, r29, r0, 0x0
beq r2, r0, end
write r2
add r1, r1, r0, 1
br loop
end:
add  r1, r0, r0, 10
write r1
add r1, r0, r0, 2
sysenter
