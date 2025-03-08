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
  mov rcx, qword [rdi + (8 * 1)]
  mov rdx, qword [rdi + (8 * 2)]
  mov rsi, qword [rdi + (8 * 3)]
  mov rbp, qword [rdi + (8 * 5)]
  mov r8,  qword [rdi + (8 * 6)]
  mov r9,  qword [rdi + (8 * 7)]
  mov r10, qword [rdi + (8 * 8)]
  mov r11, qword [rdi + (8 * 9)]
  mov r12, qword [rdi + (8 * 10)]
  mov r13, qword [rdi + (8 * 11)]
  mov r14, qword [rdi + (8 * 12)]
  mov r15, qword [rdi + (8 * 13)]

  push rdi
  mov rdi, qword [rdi + 0x20]
  mov rax, qword [rsp + 8]

  vmrun 
  ; not restoring rax right now
  push rdi
  mov rdi, qword [rsp + 8]

  mov qword [rdi], rbx
  mov qword [rdi + (8 * 1)], rcx
  mov qword [rdi + (8 * 2)], rdx
  mov qword [rdi + (8 * 3)], rsi
  mov qword [rdi + (8 * 5)], rbp
  mov qword [rdi + (8 * 6)], r8
  mov qword [rdi + (8 * 7)], r9
  mov qword [rdi + (8 * 8)], r10
  mov qword [rdi + (8 * 9)], r11
  mov qword [rdi + (8 * 10)], r12
  mov qword [rdi + (8 * 11)], r13
  mov qword [rdi + (8 * 12)], r14
  mov qword [rdi + (8 * 13)], r15

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

