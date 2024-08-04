#include "pmm.h"
#include "limine.h"
#include <cstdint>
#include <math.h>
#include "common.h"

static page_frame_allocator page_frame_allocator_struct;

//static uint8_t* page_frame_bitmap = 0;

void init_pmm(struct limine_memmap_response* memmap_response) {
  uint64_t highest_address = 0;
  for(uint32_t entry = 0; entry < memmap_response->entry_count; entry++) {
    struct limine_memmap_entry* curr_entry = memmap_response->entries[entry];
    if(highest_address < (curr_entry->base + curr_entry->length)) {
      highest_address = curr_entry->base + curr_entry->length;
    }
  }

  uint32_t needed_size = highest_address / 8;
  for(uint32_t entry = 0; entry < memmap_response->entry_count; entry++) {
    struct limine_memmap_entry* curr_entry = memmap_response->entries[entry];
    if(curr_entry->length >= needed_size && curr_entry->type == LIMINE_MEMMAP_USABLE) {
      page_frame_allocator_struct.bitmap = (uint8_t*)TO_HIGHER_HALF(curr_entry->base);
      page_frame_allocator_struct.bitmap_size = needed_size;
      page_frame_allocator_struct.entry = memmap_response;
    }
  }

  for(uint32_t entry = 0; entry < memmap_response->entry_count; entry++) {
    struct limine_memmap_entry* curr_entry = memmap_response->entries[entry];
    if(curr_entry->type == LIMINE_MEMMAP_USABLE) {
      for(uint32_t i = 0; i < (curr_entry->length / 0x1000); i++) {
        uint64_t addr = curr_entry->length + 0x1000 * i;
        uint32_t byte_idx = addr / 8;
        uint8_t bit_idx = addr % 8;
        setbit((uint64_t*)&page_frame_allocator_struct.bitmap[byte_idx], bit_idx);
      }
    }
  }
}

uint64_t kpalloc() {
  for(uint32_t i = 0; i < page_frame_allocator_struct.bitmap_size; i++) {
    for(uint8_t j = 0; j < 8; j++) {
      if(getbit(page_frame_allocator_struct.bitmap[i], j) == 1) {
        setbit((uint64_t*)&page_frame_allocator_struct.bitmap[i], j);
        return (uint64_t)((i+j)*0x1000);
      }
    }
  }

  return 0;
}

uint64_t kpalloc_contignious(uint32_t count) {
  for(uint32_t i = 0; i < page_frame_allocator_struct.bitmap_size; i++) {
    for(uint8_t j = 0; j < 8; j++) {
      if(getbit(page_frame_allocator_struct.bitmap[i], j) == 1) {
        uint32_t z = 1;
        for(z = 1; z < count; z++) {
          if(getbit(page_frame_allocator_struct.bitmap[i], j+z) == 0) {
            break;
          } else {
            continue;
          }
        }

        if(z == count) {
          return (uint64_t)((i+j)*0x1000);
        }
      }
    }
  }

  return 0;
}

void kpfree(uint64_t page) {
  uint32_t byte_idx = (page / 0x1000) / 8;
  uint32_t bit_idx = (page / 0x1000) % 8;
  clearbit((uint64_t*)&page_frame_allocator_struct.bitmap[byte_idx], bit_idx);
}
