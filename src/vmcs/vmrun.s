bits 64
section .text

global svm_vmrun
svm_vmrun:
  push rbp
  push rbx
  push r12
  push r13
  push r14
  push r15

  push rsi

  mov rbx, qword [rdi]
  mov rcx, qword [rdi + 8]
  mov rdx, qword [rdi + 16]
  mov rsi, qword [rdi + 24]
  mov rbp, qword [rdi + 32]
  mov r8, qword [rdi + 40]
  mov r9, qword [rdi + 48]
  mov r10, qword [rdi + 56]
  mov r11, qword [rdi + 64]
  mov r12, qword [rdi + 72]
  mov r13, qword [rdi + 80]
  mov r14, qword [rdi + 88]
  mov r15, qword [rdi + 96]

  push rdi
  mov rdi, qword [rdi + 0x20]
  mov rax, qword [rsp + 8]

  vmrun 

  push rdi
  mov rdi, qword [rsp + 8]

  mov qword [rdi], rbx
  mov qword [rdi + 8], rcx
  mov qword [rdi + 16], rdx
  mov qword [rdi + 24], rsi
  mov qword [rdi + 32], rbp
  mov qword [rdi + 40], r8
  mov qword [rdi + 48], r9
  mov qword [rdi + 56], r10
  mov qword [rdi + 64], r11
  mov qword [rdi + 72], r12
  mov qword [rdi + 80], r13
  mov qword [rdi + 88], r14
  mov qword [rdi + 96], r15

  pop r8
  pop r9
  mov qword [r9 + 0x20], r8

  pop rsi

  pop r15
  pop r14
  pop r13
  pop r12
  pop rbx
  pop rbp

  ret

