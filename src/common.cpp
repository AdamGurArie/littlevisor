#include "common.h"
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

//template<typename T>
//uint8_t read_from_port_byte(uint16_t port) {
//  uint8_t output = 0;
//  asm volatile("inb %0, %1" : "=r"(output) : "r"(port));
//  return output;
//}

//template void write_to_port<uint8_t>(uint16_t port, uint8_t data); 

