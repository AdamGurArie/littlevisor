#include <bit>
#include <cstdint>
#include <cstring>

#include "elfloader.h"
#include "../pmm.h"
#include "../mm/npaging.h"

#define ELF_MAGIC_SEQ 0x7f454C46
#define ELF_LINK_BASE_ADDR 0x10000

void load_elf_to_memory() {
/*  elfhdr* header = std::bit_cast<header*>(ELF_LINK_BASE_ADDR);
  uint32_t elf_magic_seq = ELF_MAGIC_SEQ;
  if(!kmemcmp(header->e_ident, &elf_magic_seq, 4)) {
    // abort elf loading
  }

  phdr* phdr_list = std::bit_cast<phdr*>(ELF_LINK_BASE_ADDR + header->e_phoff);
  for(uint32_t i = 0; i < header->e_phnum; i++) {
    phdr curr_hdr = phdr_list[i];
    if(curr_hdr.p_type != PT_LOAD) {
      continue;
    }

    uint64_t page_offset = 0;
    while(page_offset < curr_hdr.p_memsz) {
      uint64_t physical_page = kpalloc();
      mapPage(physical_page, curr_hdr.p_vaddr + page_offset, 0x07);
      memset((uint8_t*)(curr_hdr.p_vaddr + page_offset), 0x0, 0x1000);
      memcpy((uint8_t*)(curr_hdr.p_vaddr + page_offset), (uint8_t*)(ELF_LINK_BASE_ADDR + curr_hdr.p_offset + page_offset), 0x1000);
      page_offset+=0x1000;
    }
  }*/
}
