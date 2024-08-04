#pragma once

#include <cstdint>
struct elfhdr {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} __attribute__((packed));

struct phdr {
  uint32_t p_type;
  uint32_t p_offset;
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz;
  uint32_t p_memsz;
  uint32_t p_flags;
  uint32_t p_align;
};

enum p_types {
  PT_NULL,
  PT_LOAD,
  PT_DYNAMIC,
  PT_INTERP,
  PT_NOTE,
  PT_SHLIB,
  PT_PHDR,
  PT_LOPROC = 0x70000000,
  PT_HIPROC = 0x7fffffff
};

void load_elf_to_memory();
