#pragma once
#include <cstdint>
#include <stdint.h>

#include "common.h"

struct idt_entry {
  uint16_t offset_1;
  uint16_t seg_selector;
  uint8_t ist : 3;
  uint8_t reserved_1 : 5;
  uint8_t type : 4;
  uint8_t reserved_2 : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint16_t offset_2;
  uint32_t offset_3;
  uint32_t reserved_3;
} __attribute__((packed));

struct idtr_reg {
  uint16_t limit;
  uint64_t base_addr;
} __attribute__((packed));

void init_idtentry(uint16_t vector, uint64_t isr_addr, uint8_t dpl, uint8_t ist, uint8_t type);

void loadidt();

void init_idt();

void page_fault_handler(Stack* stack);
