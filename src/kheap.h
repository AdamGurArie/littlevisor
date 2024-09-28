#pragma once
#include <cstdint>

void* kmalloc(uint32_t size);
void kfree(void* ptr);
