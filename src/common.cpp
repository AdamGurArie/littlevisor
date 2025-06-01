#include "common.h"
#include "kheap.h"
#include <cstdint>

template<typename T>
void write_to_port(uint16_t port, T data) {
  asm volatile("out %0, %1" :: "r"(data), "r"(port));
}

template void write_to_port(uint16_t port, uint8_t data);
template void write_to_port(uint16_t port, uint16_t data);
template void write_to_port(uint16_t port, uint32_t data);

uint8_t read_from_port_byte(uint16_t port) {
  uint8_t data = 0;
  asm volatile("inb %1, %0" : "=r"(data) : "Nd"(port));
  
  return data;
}

uint16_t read_from_port_word(uint16_t port) {
  uint16_t data = 0;
  asm volatile("inw %1, %0" : "=r"(data) : "Nd"(port));

  return data;
}

uint32_t read_from_port_dword(uint16_t port) {
  uint32_t data = 0;
  asm volatile("inl %1, %0" : "=r"(data) : "Nd"(port));
  
  return data;
}

void print_to_serial(uint8_t* text, int size) {
  for(int i = 0; i < size; i++) {
    write_to_port(0xe9, text[i]);
  }
}

/*uint64_t strlen(const char *str) {
    const char *s = str;
    while (*s) {
        s++;
    }
    return s - str;
}*/

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

void swap(char *x, char *y){
    char t = *x; *x = *y; *y = t;
}

char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}

void itoa(long value, char* buffer){
    uint64_t n = value;
 
    uint64_t i = 0;
    while (n)
    {
        uint64_t r = n % 10;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / 10;
    }
 

    if (i == 0) {
        buffer[i++] = '0';
    }

    if (value < 0 && 10 == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0';
 
    reverse(buffer, 0, i - 1);
}
