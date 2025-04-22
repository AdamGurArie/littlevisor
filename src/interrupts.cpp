#include "interrupts.h"
#include "common.h"
#include <cstdint>
#include "gdt.h"

#define PIC1_DATA 0x21
#define PIC1_CMD 0x20

#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

extern "C" void general_isr_tram();
extern "C" void pagefault_handler_tram();
extern "C" void isr_pit_handler();
extern void* isr_stub_table[];

static struct idt_entry idt_table[256];

void init_idt_entry(uint16_t vector, uint64_t isr_addr, uint8_t dpl, uint8_t ist, uint8_t type) {
  idt_table[vector].dpl = dpl;
  idt_table[vector].ist = ist;
  idt_table[vector].type = type;
  idt_table[vector].present = 1;
  idt_table[vector].seg_selector = CODE64_DESC_SELECTOR;
  idt_table[vector].offset_1 = isr_addr & 0xFFFF;
  idt_table[vector].offset_2 = (isr_addr >> 16) & 0xFFFF;
  idt_table[vector].offset_3 = (isr_addr >> 32) & 0xFFFFFFFF;
  idt_table[vector].reserved_1 = 0;
  idt_table[vector].reserved_2 = 0;
  idt_table[vector].reserved_3 = 0;
}

void loadidt() {
  struct idtr_reg idtr {
    .limit = sizeof(idt_entry)*256,
    .base_addr = (uint64_t)&idt_table,
  };

  asm __volatile__("lidt %0" :: "m"(idtr));
  asm __volatile__("sti");
}

void init_idt() {
  for(int i = 0; i < 32; i++) {
    init_idt_entry(i, (uint64_t)isr_stub_table[i], 0, 0, 0xF);
  }
  // for(int i = 0; i < 32; i++) {
  //   init_idtentry(i, (uint64_t)general_isr_tram, 0, 0, 0xF);
  // }

  init_idt_entry(0xE, (uint64_t)pagefault_handler_tram, 0, 0, 0xF);
  //init_idt_entry(0x20, (uint64_t)isr_pit_handler, 0, 0, 0xE);
  //init_idtentry(0xD, (uint64_t)page_fault_handler, 0, 0, 0xF);
  //init_idtentry(0x6, (uint64_t)page_fault_handler, 0, 0, 0xF);

  loadidt();

  pic_remap(0x20, 0x28);
  for(uint32_t irq_line = 0; irq_line < 16; irq_line++) {
    irq_set_mask(irq_line);
  }
}

void general_isr(uint8_t isr_vector) {
  write_to_port(0xe9, (uint8_t)isr_vector);
  while(true);
}

void page_fault_handler(uint8_t isr_vector, Stack* stack) {
  // read cr2
  (void)stack;
  // uint64_t fault_address = 0;
  // asm volatile("mov %0, %%cr2" : "=r"(fault_address));
  // if(fault_address > 0x1F7 && fault_address < (0x1FE)) {
    //ide
  // }

  while(1);
}

void irq_clear_mask(uint8_t irq_line) {
  uint16_t port = 0;
  uint8_t value = 0;

  if(irq_line < 8) {
    port = PIC1_DATA;
  } else if(irq_line > 8 && irq_line < 16) {
    port = PIC2_DATA;
    irq_line -= 8;
  } else {
    return;
  }

  value = read_from_port_byte(port) & ~(1 << irq_line);
  write_to_port(port, value);
}

void irq_set_mask(uint8_t irq_line) {
  uint16_t port = 0;
  uint8_t value = 0;

  if(irq_line < 8) {
    port = PIC1_DATA;
  } else if(irq_line > 8 && irq_line < 16) {
    port = PIC2_DATA;
    irq_line -= 8;
  } else {
    return;
  }

  value = read_from_port_byte(port) | (1 << irq_line);
  write_to_port(port, value);
}

void pic_remap(uint32_t offset1, uint32_t offset2) {
  write_to_port((uint16_t)PIC1_CMD, (uint8_t)(ICW1_INIT | ICW1_ICW4));
  write_to_port((uint16_t)PIC2_CMD, (uint8_t)(ICW1_INIT | ICW1_ICW4));
  write_to_port((uint16_t)PIC1_DATA, (uint8_t)offset1);
  write_to_port((uint16_t)PIC2_DATA, (uint8_t)offset2);
  write_to_port((uint16_t)PIC1_DATA, (uint8_t)0x4);
  write_to_port((uint16_t)PIC2_DATA, (uint8_t)0x2);

  write_to_port((uint16_t)PIC1_DATA, (uint8_t)ICW4_8086);
  write_to_port((uint16_t)PIC2_DATA, (uint8_t)ICW4_8086);

  write_to_port((uint16_t)PIC1_DATA, (uint8_t)0x0);
  write_to_port((uint16_t)PIC2_DATA, (uint8_t)0x0);
}
