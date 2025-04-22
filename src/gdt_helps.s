bits 64

global setGdt

gdtr DW 0 
     DQ 0

; @TODO: triggers a GPF, debug
setGdt:
  lgdt [rdi]
  ;ret
  lea rax, [rel .flush]
  push 0x28
  push rax
  retfq

  .flush:
    mov ax, 0x30
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov es, ax
    ret
