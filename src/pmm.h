#pragma once
#include "limine.h"

struct page_frame_allocator {
  limine_memmap_response* entry;
  uint8_t* bitmap; 
  uint32_t bitmap_size;
  uint32_t last_allocated_idx;
};

struct bitmap_entry {
  uint8_t* ptr;
  uint32_t size;
  uint8_t available;
};

void init_pmm(struct limine_memmap_response* memmap_response);
uint32_t get_needed_bitmap_size(limine_memmap_entry** entry_list, uint32_t length);
uint64_t kpalloc();                 // kp = kernel-physical;)
uint64_t kpalloc_contignious(uint32_t count);
void kpfree(uint64_t page);
