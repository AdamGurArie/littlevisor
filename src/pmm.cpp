#include "pmm.h"
#include "limine.h"
#include <cstdint>
#include <math.h>

static page_frame_allocator page_frame_allocator_struct = {0};

static bitmap_entry* page_frame_bitmap = 0;

void init_pmm(struct limine_memmap_response* memmap_response) {
  struct limine_memmap_entry** entry_list = memmap_response->entries;
  uint32_t bitmap_size = get_needed_bitmap_size(entry_list, memmap_response->entry_count);
  uint8_t* bitmap = 0;
  uint32_t bitmap_index = 0;

  // find a place for the bitmap
  for(uint32_t i = 0; i < memmap_response->entry_count; i++) {
    struct limine_memmap_entry* entry = memmap_response->entries[i];
    if(entry->type == LIMINE_MEMMAP_USABLE) {
      if(entry->length >= bitmap_size) {
        page_frame_bitmap = (bitmap_entry*)entry->base;
      }
    }
  }

  if(page_frame_bitmap == 0) {
    //panic
  }

  // create the bitmap
  for(uint32_t i = 0; i < memmap_response->entry_count; i++) {
    struct limine_memmap_entry* entry = memmap_response->entries[i];
   
    if(entry->type == LIMINE_MEMMAP_USABLE) {
      
      for(uint32_t j = 0; j < entry->length / 0x1000; j++) {
        
        if(entry->base == (uint64_t)bitmap) {
          uint32_t num_of_pages = bitmap_size / 0x1000;
          
          for(uint32_t page_idx = 0; page_idx < num_of_pages; page_idx++) {
            page_frame_bitmap[bitmap_index].available = 0;
            page_frame_bitmap[bitmap_index].ptr = (uint8_t*)(entry->base + j * 0x1000);
            page_frame_bitmap[bitmap_index].size = (entry->length - j * 0x1000) > 0x1000 ? 0x1000 : entry->length - j * 0x1000;
            bitmap_index++;
            j++;  //might cause a problem
          }

        } else {
          bitmap[bitmap_index] = 0;
          bitmap_index++;
        }
      }
   }
 }

 page_frame_allocator_struct.bitmap = bitmap;
 page_frame_allocator_struct.bitmap_size = bitmap_size;
 page_frame_allocator_struct.entry_list = memmap_response->entries;
}

uint32_t get_needed_bitmap_size(limine_memmap_entry** entry_list, uint32_t length) {
  uint32_t needed_size = 0;
  for(uint32_t i = 0; i < length; i++) {
    limine_memmap_entry* entry = entry_list[i];
    if(entry->type == LIMINE_MEMMAP_USABLE) {
      needed_size += ceil(entry->length / 0x1000);
    }
  }

  return needed_size * 13;
}

uint64_t kpalloc(uint32_t size) {
  for(uint32_t i = 0; i < page_frame_allocator_struct.bitmap_size; i++) {
    if(page_frame_bitmap->available == 0
       && page_frame_bitmap->size >= size) {
      page_frame_bitmap->available = 1;
      return (uint64_t)page_frame_bitmap->ptr;
      
    }
  }

  return 0;
}

void kpfree(uint64_t page) {
  for(uint32_t i = 0; i < page_frame_allocator_struct.bitmap_size; i++) {
    if((uint64_t)page_frame_bitmap[i].ptr == page) {
      page_frame_bitmap[i].available = 0;
      return;
    }
  }
}
