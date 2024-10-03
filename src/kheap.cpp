#include "kheap.h"
#include <cstdint>
#include "common.h"
#include "pmm.h"

enum CHUNK_STATE {
  FREE,
  OCCUPIED
};

struct FREELIST_CHUNK {
  FREELIST_CHUNK* next_chunk;
  CHUNK_STATE state;
};

static FREELIST_CHUNK* first_chunk;
static FREELIST_CHUNK* last_free_chunk;

static void checkConsolidate(FREELIST_CHUNK* chunk);

void init_heap() {
  uint64_t heap_space = TO_HIGHER_HALF(kpalloc_contignious(4));
  if(heap_space == 0) {
    // need to create a mechanism like kpanic() for such cases
    return;
  }

  first_chunk = (FREELIST_CHUNK*)heap_space;
  first_chunk->next_chunk = (FREELIST_CHUNK*)(heap_space + 4 * 0x1000);
  last_free_chunk = first_chunk;
}

void* kmalloc(uint64_t size) {
  uint64_t remaining_size = (uint64_t)last_free_chunk->next_chunk - (uint64_t)last_free_chunk;
  if (size + sizeof(FREELIST_CHUNK) > remaining_size) {
    return 0;
  }

  uint64_t chunk_addr = (uint64_t)last_free_chunk + sizeof(FREELIST_CHUNK);
  FREELIST_CHUNK* new_chunk = (FREELIST_CHUNK*)(chunk_addr + size);
  new_chunk->state = FREE;
  new_chunk->next_chunk = last_free_chunk->next_chunk;
  last_free_chunk->state = OCCUPIED;
  last_free_chunk = new_chunk;

  return (void*)chunk_addr;
}

void kfree(void* addr) {
  FREELIST_CHUNK* curr_chunk = first_chunk;
  while(curr_chunk != (FREELIST_CHUNK*)0x0) {

    if((uint64_t)curr_chunk + sizeof(FREELIST_CHUNK) == (uint64_t)addr) {

      curr_chunk->state = FREE;
      checkConsolidate(curr_chunk);

      return;
    }
  }

  return;
}

void checkConsolidate(FREELIST_CHUNK* chunk) {
  if((uint64_t)chunk == 0) {
    return;
  }

  if((uint64_t)chunk->next_chunk == 0) {
    return;
  }

  if(chunk->next_chunk->state == FREE) {
    chunk->next_chunk = chunk->next_chunk->next_chunk;
  }

  return;
}
