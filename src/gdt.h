#pragma once

#include <cstdint>

#define NULL_DESC_SELECTOR 0x0
#define CODE16_DESC_SELECTOR 0x8
#define DATA16_DESC_SELECTOR 0x10
#define CODE32_DESC_SELECTOR 0x18
#define DATA32_DESC_SELECTOR 0x20
#define CODE64_DESC_SELECTOR 0x28
#define DATA64_DESC_SELECTOR 0x30
#define CODE64_USER_DESC_SELECTOR 0x38
#define DATA64_USER_DESC_SELECTOR 0x40

struct gdt_entry {
  uint16_t limit_1;
  uint16_t base_1;
  uint8_t base_2;
  uint8_t access;
  uint8_t limit_2 : 4;
  uint8_t flags : 4;
  uint8_t base_3;
} __attribute__((packed));

struct tss_entry {
  uint16_t limit_1;
  uint16_t base_1;
  uint8_t base_2;
  uint8_t access;
  uint8_t limit_2 : 4;
  uint8_t flags : 4;
  uint8_t base_3;
  uint32_t base_4;
  uint32_t reserved;
} __attribute__((packed));

struct tss_s {
  uint32_t reserved1;
  uint32_t rsp0_low;
  uint32_t rsp0_high;
  uint32_t rsp1_low;
  uint32_t rsp1_high;
  uint32_t rsp2_low;
  uint32_t rsp2_high;
  uint32_t reserved2;
  uint32_t reserved3;
  uint32_t ist1_low;
  uint32_t ist1_high;
  uint32_t ist2_low;
  uint32_t ist2_high;
  uint32_t ist3_low;
  uint32_t ist3_high;
  uint32_t ist4_low;
  uint32_t ist4_high;
  uint32_t ist5_low;
  uint32_t ist5_high;
  uint32_t ist6_low;
  uint32_t ist6_high;
  uint32_t ist7_low;
  uint32_t ist7_high;
  uint32_t reserved4;
  uint32_t reserved5;
  uint16_t reserved6;
  uint16_t iobp;
} __attribute__((packed)); 

struct gdt_table {
  gdt_entry table[9];
  tss_entry tss_desc;
} __attribute__((packed));

struct gdtr_struct {
  uint16_t size;
  uint64_t offset;
} __attribute__((packed));

void init_gdt();

void switch_tss_userstack(uint64_t new_stack);

void switch_tss_kernelstack(uint64_t new_stack);
