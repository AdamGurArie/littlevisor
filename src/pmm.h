#pragma once
#include "limine.h"

struct page_frame_allocator {
  uint32_t bitmap_size;
  uint8_t* bitmap;
  limine_memmap_entry** entry_list;
};

struct bitmap_entry {
  uint8_t* ptr;
  uint32_t size;
  uint8_t available;
};

void init_pmm(struct limine_memmap_response* memmap_response);
uint32_t get_needed_bitmap_size(limine_memmap_entry** entry_list, uint32_t length);
uint64_t kpalloc(uint32_t number_of_pages);                 // kp = kernel-physical;)
void kpfree(uint64_t page);
