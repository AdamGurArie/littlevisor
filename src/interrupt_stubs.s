bits 64

extern _Z11general_isrv
extern _Z18page_fault_handlerP5Stack
global general_isr_tram

%macro pushAll 0
    push rsp
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popAll 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    pop rsp
%endmacro

general_isr_tram:
  push qword 0
  cld
  
  pushAll

  mov rdi, rax

  call _Z11general_isrv

  mov rsp, rax

  popAll

  add rsp, 8

  iretq


pagefault_handler_tram:
  push qword 0
  cld

  pushAll

  mod rdi, rsp

  call _Z18page_fault_handlerP5Stack

  mov rsp, rax

  popAll

  add rsp, 8

  iretq
