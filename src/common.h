#pragma once
#include <cstdint>
#include <type_traits>
#include <concepts>

#define TO_HIGHER_HALF(addr) \
  addr + 0xffff800000000000;

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

void kmemcpy(uint8_t* dest, uint8_t* src, uint32_t size);

void kmemset(uint8_t* mem, uint8_t val, uint32_t size);

uint8_t kmemcmp(uint8_t* mem1, uint8_t* mem2, uint32_t size);

inline void clearbit(uint64_t* val, uint8_t pos) {
  *val = *val & ~((uint64_t)1 << pos);
}
//template<acceptable_types T>
//T read_from_port(uint16_t port);
