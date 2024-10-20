#pragma once
#include <cctype>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <cstdint>

#define TO_HIGHER_HALF(addr) \
  (addr + 0xffff800000000000)

#define TO_LOWER_HALF(addr) \
  (addr - 0xffff800000000000)

struct Stack
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t isr_number;
    uint64_t err_code;
    uint64_t rip;
    uint64_t cs; 
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

//template<typename T>
//concept acceptable_types = std::is_same_v<T, uint8_t>  ||
//                           std::is_same_v<T, uint16_t> ||
//                           std::is_same_v<T, uint32_t>;

//template<acceptable_types T>
void write_to_port(uint16_t port, uint8_t data);
void print_to_serial(uint8_t* text, int size);

inline bool getbit(uint64_t val, uint8_t pos) {
  uint64_t bitmask = (uint64_t)1 << pos;
  return (bool)((val & bitmask) >> pos);
}

void setbit(uint64_t* val, uint8_t pos);
void kmemcpy(uint8_t* dest, const uint8_t* src, uint32_t size);
void kmemset(uint8_t* mem, uint8_t val, uint32_t size);
uint8_t kmemcmp(uint8_t* mem1, uint8_t* mem2, uint32_t size);
uint32_t kstrlen(const char* str);
int ktoupper(int c);

inline void clearbit(uint64_t* val, uint8_t pos) {
  *val = *val & ~((uint64_t)1 << pos);
}

inline void clearbit(uint8_t* val, uint8_t pos) {
  *val = *val & ~((uint8_t)1 << pos);
}

inline void setbit(uint64_t* val, uint8_t pos) {
  *val = *val | ((uint64_t)1 << pos);
}

inline uint8_t getbit(uint64_t* val, uint8_t pos) {
  return (*val >> pos) & (uint64_t)1;
}

inline uint64_t rdmsr(uint32_t msr) {
  uint32_t eax = 0;
  uint32_t edx = 0;
  asm volatile("rdmsr" : "d"(edx) "a"(eax) : "c"(msr));
  return (uint64_t)(edx >> 32) | eax;
}

inline void wrmsr(uint32_t msr, uint64_t val) {

}

void* operator new(std::size_t size);
void operator delete(void* p) noexcept;
void operator delete(void* p, uint64_t param) noexcept;
//template<acceptable_types T>
//T read_from_port(uint16_t port);
