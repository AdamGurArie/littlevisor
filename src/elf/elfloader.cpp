#include <bit>
#include <cstdint>
#include <cstring>

#include "elfloader.h"
#include "../pmm.h"
#include "../mm/npaging.h"
#include "../fs/vfs.h"
#include "../common.h"

#define ELF_MAGIC_SEQ 0x464C457F
#define ELF_LINK_BASE_ADDR 0x10000

#define PT_LOAD 0x1

uint64_t load_elf_to_memory(uint32_t elf_fd, uint64_t memmap) {
  elf_hdr elf_header;
  vreadFile(elf_fd, (char*)&elf_header, sizeof(elf_hdr));

  uint32_t elf_magic_seq = ELF_MAGIC_SEQ;

  if(kmemcmp((uint8_t*)&elf_header.e_ident, (uint8_t*)&elf_magic_seq, sizeof(uint32_t))) {
    return 0x0;
  }
  
  if(elf_header.e_phoff == 0) {
    return 0x0;
  }

  vseekp(elf_fd, elf_header.e_phoff);
  for(uint32_t hdr_idx = 0; hdr_idx < elf_header.e_phnum; hdr_idx++) {
    uint32_t offset_in_file = elf_header.e_phoff + hdr_idx * elf_header.e_phentsize;
    vseekp(elf_fd, offset_in_file);
    
    elf_phdr phdr;
    vreadFile(elf_fd, (char*)&phdr, elf_header.e_phentsize);
    
    if(phdr.p_type == PT_LOAD) {

      uint32_t num_of_pages = (phdr.p_memsz + PAGE_SIZE) / PAGE_SIZE;
      vseekp(elf_fd, phdr.p_offset);
      
      if(phdr.p_vaddr % phdr.p_align != 0) {
        uint64_t page = kpalloc();
        if(page == 0x0) {
          kpanic();
        }

        mapPage(
            page,
            phdr.p_vaddr,
            0x07,
            memmap
        );

        uint8_t* buff = (uint8_t*)kmalloc(phdr.p_vaddr % phdr.p_align);
        if(buff == 0x0) {
          kpanic();
        }

        vreadFile(elf_fd, (char*)buff, phdr.p_vaddr % phdr.p_align);
        kmemcpy(
            (uint8_t*)(TO_HIGHER_HALF(page) + phdr.p_vaddr % phdr.p_align),
            buff,
            phdr.p_vaddr % phdr.p_align
        );

        kfree(buff);
        vseekr(elf_fd, phdr.p_vaddr % phdr.p_align);
        num_of_pages--;
      }

      for(uint32_t i = 0; i < num_of_pages; i++) {
        uint64_t page = kpalloc();
        if(page == 0x0) {
          // @TODO: change this behavior
          kpanic();
        }

        mapPage(
            page,
            phdr.p_vaddr + i * PAGE_SIZE,
            0x07,
            memmap
        );

        uint8_t* buff = (uint8_t*)kmalloc(PAGE_SIZE);
        if(buff == 0x0) {
          // TODO: change this behavior
          kpanic();
        }
        
        vreadFile(elf_fd, (char*)buff, PAGE_SIZE);
        kmemcpy((uint8_t*)TO_HIGHER_HALF(page), (uint8_t*)buff, PAGE_SIZE);
        kfree(buff);
      }
    }
  }

  return elf_header.e_entry;
}
