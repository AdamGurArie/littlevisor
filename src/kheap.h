#pragma once
#include <cstdint>

void* kmalloc(uint64_t size);
void kfree(void* ptr);
