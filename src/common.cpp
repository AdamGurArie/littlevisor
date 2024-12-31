#include "common.h"
#include "kheap.h"
#include <cstdint>

//template<typename T>
void write_to_port(uint16_t port, uint8_t data) {
  asm volatile("outb %0, %1" :: "r"(data), "r"(port));
}

void print_to_serial(uint8_t* text, int size) {
  for(int i = 0; i < size; i++) {
    write_to_port(0xe9, text[i]);
  }
}

//void setbit(uint64_t* val, uint8_t pos) {
//  *val = *val | ((uint64_t)1 << pos);
//}

[[noreturn]] void kpanic() {
  print_to_serial((uint8_t*)"Kernel Panic", 12);
  while(true);
}

void kmemcpy(uint8_t* dest, const uint8_t* src, uint32_t size) {
  for(uint32_t i = 0; i < size; i++) {
    dest[i] = src[i];
  }
}

void kmemset(uint8_t* mem, uint8_t val, uint32_t size) {
  for(uint32_t i = 0; i < size; i++) {
    mem[i] = val;
  }
}

uint8_t kmemcmp(uint8_t* mem1, uint8_t* mem2, uint32_t size) {
  for(uint32_t i = 0; i < size; i++) {
    if(mem1[i] != mem2[i]) {
      return 1;
    }
  }

  return 0;
}

uint32_t kstrlen(const char* str) {
  uint32_t length = 0;
  while(str[length] != '\x00') {
    length++;
  }

  return length;
}

int ktoupper(int c) {
  if(c > 0x60 && c < 0x7B) {
    return c - 32;
  }

  return c;
}

uint32_t kToLittleEndian(uint32_t value) {
    uint64_t result = 0;
    
    for (int i = 0; i < 8; ++i) {
        result |= ((value >> (i * 8)) & 0xFF) << ((7 - i) * 8);
    }

    return result;
}

uint64_t kToLittleEndian(uint64_t value) {
  uint64_t result = 0;
  for (int i = 0; i < 8; ++i) {
    result |= ((value >> (i * 8)) & 0xFF) << ((7 - i) * 8);
  }

  return result;
}

//template<typename T>
//uint8_t read_from_port_byte(uint16_t port) {
//  uint8_t output = 0;
//  asm volatile("inb %0, %1" : "=r"(output) : "r"(port));
//  return output;
//}

//template void write_to_port<uint8_t>(uint16_t port, uint8_t data); 

void* operator new(std::size_t size) {
  return kmalloc(size); 
}

void operator delete(void* p) noexcept {
  kfree(p);
}

void operator delete(void* p, uint64_t param) noexcept {
  (void)param;
  kfree(p);
}
