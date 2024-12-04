bits 64

extern _Z11general_isrh
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
  cld
  
  pushAll
  
  mov rax, rsp
  mov rdi, rsp

  call _Z11general_isrh

  mov rsp, rax

  popAll

  iretq


pagefault_handler_tram:
  push qword 0
  cld

  pushAll

  mov rdi, rsp

  call _Z18page_fault_handlerP5Stack

  mov rsp, rax

  popAll

  add rsp, 8

  iretq


%macro isr_err_stub 1
isr_stub_%+%1:
    mov rdi, %1
    call _Z11general_isrh
    iretq
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    mov rdi, %1
    call _Z11general_isrh
    iret
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep
