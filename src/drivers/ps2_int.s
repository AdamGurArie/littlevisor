global isr_kbd_handler 
extern _Z19handle_keyboard_intv

isr_kbd_handler:
  cli

  push 0
  push 33
  pushAll
  mov rdi, rsp
  cld
  call _Z19handle_keyboard_intv
  mov al, 0x20
  out 0x20, al
  out 0xA0, al
  popAll
  add rsp, 16
  
  sti
  iretq
