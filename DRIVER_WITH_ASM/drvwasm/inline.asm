
;this file contains the code that runs instead of the default subroutine that is used by Windows to treat a system call

;extern variables defined in driver's source code
EXTERN TargetProcessId:QWORD
EXTERN fake_msr:QWORD

; pointers to functions used to write to the user stack of the thread being redirected
EXTERN afake_return_address:QWORD
;EXTERN astack_pointer:QWORD
EXTERN zwprotect:QWORD
EXTERN MYMmUnlockPages:QWORD
EXTERN MYIoAllocateMdl:QWORD
EXTERN MYIoFreeMdl:QWORD
EXTERN MYMmMapLockedPagesSpecifyCache:QWORD
EXTERN MYMmProbeAndLockPages:QWORD
;EXTERN aoriginal_return:QWORD  
EXTERN MYKeLowerIrql:QWORD
EXTERN MYMmProtectMdlSystemAddress:QWORD
EXTERN MYKeRaiseIrqlToDpcLevel:QWORD

.data

.code _text
save_registers MACRO 

   push rax      ;save current rax
   push rbx      ;save current rbx
 ;  push rcx      ;save current rcx
   push rdx      ;save current rdx
   push rbp      ;save current rbp
   push rdi      ;save current rdi
   push rsi      ;save current rsi
   push r8         ;save current r8
   push r9         ;save current r9
   push r10      ;save current r10
   push r11      ;save current r11
   push r12      ;save current r12
   push r13      ;save current r13
   push r14      ;save current r14
   push r15      ;save current r15

ENDM

restore_registers MACRO

   pop r15         ;restore current r15
   pop r14         ;restore current r14
   pop r13         ;restore current r13
   pop r12         ;restore current r12
   pop r11         ;restore current r11
   pop r10         ;restore current r10
   pop r9         ;restore current r9
   pop r8         ;restore current r8
   pop rsi         ;restore current rsi
   pop rdi         ;restore current rdi
   pop rbp         ;restore current rbp
   pop rdx         ;restore current rdx
 ;  pop rcx         ;restore current rcx
   pop rbx         ;restore current rbx
   pop rax         ;restore current rax
ENDM


;subroutine that reimplements the transition from user to kernel space normally donw when tranfering execution flow
;to subroutin ein LSTAR register

MainAsm PROC PUBLIC
swapgs
mov qword ptr gs:[10h],rsp
mov rsp,qword ptr gs:[1a8h] 
push fake_msr
call GetTidAsm
ret
MainAsm ENDP 



GetTidAsm PROC PUBLIC
push rax
push rbx
pushfq
cmp rax,1000h
jge NOT_EQ
mov rax, qword ptr gs:[188h]
mov rax, qword ptr [rax+630h]
cmp rax, TargetProcessId
jne NOT_EQ

save_registers

;from this line downwards is the assembly version of WorkerHook() funciton in Communication.c
;We use the assembly representation because we need to use the kernel stack of each thread to store local variables

mov rax, qword ptr gs:[10h]
sub rsp, 0C8h
 mov qword ptr [rsp+60h], rax; stack pointer
mov qword ptr [rsp+98h], rcx; original_return
 mov qword ptr [rsp+48h], 0
 mov qword ptr [rsp+58h], 0
mov qword ptr [rsp+0A0h], 0
 mov rax, qword ptr [rsp+60h]
 and rax, 0FFFFFFFFFFFFF000h
 mov qword ptr [rsp+68h], rax
 mov rax, 012345h;cfg
 mov qword ptr [rsp+70h], rax
 mov rax, qword ptr [rsp+70h]
 call MYKeRaiseIrqlToDpcLevel
 mov byte ptr [rsp+40h], al
 mov rax, 012345h;cfg
 mov qword ptr [rsp+78h], rax
 mov qword ptr [rsp+20h], 0
 xor r9d, r9d
 xor r8d, r8d
 mov edx, 1000h
 mov rcx, qword ptr [rsp+68h]
 mov rax, qword ptr [rsp+78h]
 call MYIoAllocateMdl
 mov qword ptr [rsp+48h], rax
 cmp qword ptr [rsp+48h], 0
 je ioalloc_fail
 mov rax, 012345h;cfg
 mov qword ptr [rsp+80h], rax
 mov r8d, 2
 xor edx, edx
 mov rcx, qword ptr [rsp+48h]
 mov rax, qword ptr [rsp+80h]
 call MYMmProbeAndLockPages
 mov rax, 012345h;cfg
 mov qword ptr [rsp+88h], rax
 mov dword ptr [rsp+28h], 20h
 mov dword ptr [rsp+20h], 0
 xor r9d, r9d
 mov r8d, 1
 xor edx, edx
 mov rcx, qword ptr [rsp+48h]
 mov rax, qword ptr [rsp+88h]
 call MYMmMapLockedPagesSpecifyCache
 mov qword ptr [rsp+58h], rax
 mov rax, 012345h;cfg
 mov qword ptr [rsp+90h], rax
 mov edx, 4
 mov rcx, qword ptr [rsp+48h]
 mov rax, qword ptr [rsp+90h]
 call MYMmProtectMdlSystemAddress
 mov dword ptr [rsp+50h], eax
 cmp qword ptr [rsp+58h], 0
 je protect_fail
 cmp dword ptr [rsp+50h], 0
 jl protect_fail
 mov rax, qword ptr [rsp+60h]
 sub rax, 8
 sub rax, qword ptr [rsp+68h]
 mov rcx, qword ptr [rsp+58h]
 mov rdx, qword ptr [rsp+98h]
 mov qword ptr [rax+rcx], rdx
mov qword ptr [rsp+0A0h], 1
protect_fail:
 mov rax, 012345h;cfg
 mov qword ptr [rsp+0A8h], rax
 mov rcx, qword ptr [rsp+48h]
 mov rax, qword ptr [rsp+0A8h]
 call MYMmUnlockPages
 mov rax, 012345h;cfg
 mov qword ptr [rsp+0B0h], rax
 mov rcx, qword ptr [rsp+48h]
 mov rax, qword ptr [rsp+0B0h]
 call MYIoFreeMdl
ioalloc_fail:
 mov rax, 012345h;cfg
 mov qword ptr [rsp+0B8h], rax
 movzx ecx, byte ptr [rsp+40h]
 mov rax, qword ptr [rsp+0B8h]
 call MYKeLowerIrql
 mov rax, qword ptr [rsp+0A0h]
 cmp rax,0
 je fail
 mov rcx, afake_return_address;success
 jmp last
 fail:
 mov rcx,qword ptr [rsp+98h]; fail
 last:
 add rsp, 0C8h
 
 restore_registers

;end process specific instrumentation
NOT_EQ:
popfq
pop rbx
pop rax
ret
GetTidAsm ENDP
END