#include "gdt.h"
#include <cstdint>

extern "C" void setGdt(uint64_t gdtr_addr);

static gdt_table table{0};
static gdtr_struct gdtr{0};

static void create_gdt_entry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

void create_gdt_entry(uint8_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
  table.table[idx].base_1 = base & 0xFFFF;
  table.table[idx].base_2 = (base << 16) & 0xFF;
  table.table[idx].base_3 = (base << 24) & 0xFF;
  table.table[idx].limit_1 = limit & 0xFFFF;
  table.table[idx].limit_2 = (limit << 16) & 0xFF;
  table.table[idx].access = access;
  table.table[idx].flags = flags;
}

void init_gdt() {
  // NULL DESCRIPTOR
   table.table[0].base_1 = 0;
   table.table[0].limit_1 = 0;
   table.table[0].limit_2 = 0;
   table.table[0].base_2 = 0;
   table.table[0].base_3 = 0;
   table.table[0].access = 0;
   table.table[0].flags = 0;

  // 16BIT CODE READABLE
  table.table[1].base_1 = 0;
  table.table[1].base_2 = 0;
  table.table[1].base_3 = 0;
  table.table[1].limit_1 = 0xFFFF;
  table.table[1].limit_2 = 0;
  table.table[1].access = 0x9A;
  table.table[1].flags = 0;

  // 16BIT DATA WRITEABLE
  table.table[2].base_1 = 0;
  table.table[2].base_2 = 0;
  table.table[2].base_3 = 0;
  table.table[2].limit_1 = 0xFFFF;
  table.table[2].limit_2 = 0;
  table.table[2].access = 0x92;
  table.table[2].flags = 0;

  // 32BIT CODE READABLE
  table.table[3].base_1 = 0;
  table.table[3].base_2 = 0;
  table.table[3].base_3 = 0;
  table.table[3].limit_1 = 0xFFFF;
  table.table[3].limit_2 = 0xF;
  table.table[3].access = 0x9A;
  table.table[3].flags = 0xC;

  // 32BIT DATA WRITEABLE
  table.table[4].base_1 = 0;
  table.table[4].base_2 = 0;
  table.table[4].base_3 = 0;
  table.table[4].limit_1 = 0xFFFF;
  table.table[4].limit_2 = 0xF;
  table.table[4].access = 0x92;
  table.table[4].flags = 0xC;

  // 64BIT CODE READABLE
  table.table[5].base_1 = 0;
  table.table[5].base_2 = 0;
  table.table[5].base_3 = 0;
  table.table[5].limit_1 = 0;
  table.table[5].limit_2 = 0;
  table.table[5].access = 0x9A;
  table.table[5].flags = 0x2;

  // 64BIT DATA WRITEABLE
  table.table[6].base_1 = 0;
  table.table[6].base_2 = 0;
  table.table[6].base_3 = 0;
  table.table[6].limit_1 = 0;
  table.table[6].limit_2 = 0;
  table.table[6].access = 0x92;
  table.table[6].flags = 0xC;

  // 64BIT USER CODE READABLE
  table.table[7].base_1 = 0;
  table.table[7].base_2 = 0;
  table.table[7].base_3 = 0;
  table.table[7].limit_1 = 0;
  table.table[7].limit_2 = 0;
  table.table[7].access = 0xF2;
  table.table[7].flags = 0x2;

  // 64BIT USER DATA WRITEABLE
  table.table[8].base_1 = 0;
  table.table[8].base_2 = 0;
  table.table[8].base_3 = 0;
  table.table[8].limit_1 = 0;
  table.table[8].limit_2 = 0;
  table.table[8].access = 0xFA;
  table.table[8].flags = 0x2;

  gdtr.size = sizeof(gdt_table) - 1;
  gdtr.offset = (uint64_t)&table;

  setGdt((uint64_t)&gdtr);
}
