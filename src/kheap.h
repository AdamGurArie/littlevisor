#pragma once
#include <cstdint>

void init_heap();
void* kmalloc(uint64_t size);
void kfree(void* ptr);
