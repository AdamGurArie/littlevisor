#pragma once
#include <cstdint>
#include <type_traits>
#include <concepts>

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

inline uint64_t setbit(uint64_t val, uint8_t pos) {
  return val | ((uint64_t)1 << pos);
}
//template<acceptable_types T>
//T read_from_port(uint16_t port);
