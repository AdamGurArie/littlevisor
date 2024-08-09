#include "interrupts.h"
#include "common.h"
#include <cstdint>

#define NULL_DESC_SELECTOR 0x0
#define CODE16_DESC_SELECTOR 0x8
#define DATA16_DESC_SELECTOR 0x10
#define CODE32_DESC_SELECTOR 0x18
#define DATA32_DESC_SELECTOR 0x20
#define CODE64_DESC_SELECTOR 0x28
#define DATA64_DESC_SELECTOR 0x30

extern "C" void general_isr_tram();

static struct idt_entry idt_table[256];

void init_idtentry(uint16_t vector, uint64_t isr_addr, uint8_t dpl, uint8_t ist, uint8_t type) {
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
    .limit = sizeof(idt_entry)*31,
    .base_addr = (uint64_t)&idt_table,
  };

  asm __volatile__("lidt %0" :: "m"(idtr));
  asm __volatile__("sti");
}

void init_idt() {
  for(int i = 0; i < 32; i++) {
    init_idtentry(i, (uint64_t)general_isr_tram, 0, 0, 0xF);
  }

  loadidt();
}

void general_isr() {
  write_to_port(0xe9, (uint8_t)'r');
  while(true);
}

void page_fault_handler(Stack* stack) {
  // read cr2
  (void)stack;
  uint64_t fault_address = 0;
  asm volatile("mov %0, %%cr2" : "=r"(fault_address));
  if(fault_address > 0x1F7 && fault_address < (0x1FE)) {
    //ide
  }
}
