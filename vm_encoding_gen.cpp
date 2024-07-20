/*#include <cstdint>
#include <iostream>
#include <iterator>
#include <ostream>

struct component_encoding {
  uint8_t access_type : 1;
  uint16_t index      : 9;
  uint8_t type        : 2;
  uint8_t reserved1   : 1;
  uint8_t width       : 2;
  uint16_t reserved2;

  operator uint32_t() {
    // return (access_type | index << 1 | type << 9 | reserved1 << 11 | width << 13 | reserved2 << 15);
    uint32_t number = 0;
    number |= access_type << 0;
    number |= index << 1;
    number |= type << 10;
    number |= reserved1 << 12;
    number |= width << 13;
    number |= reserved2 << 15;
    return number;
  }
} __attribute__((packed));


int main() {
  std::cout << "enter access_type" << std::endl;
  uint32_t access_type = 0;
  std::cin >> access_type;
  std::cout << "enter index" << std::endl;
  uint32_t index = 0;
  std::cin >> index;
  std::cout << "enter type" << std::endl;
  uint32_t type = 0;
  std::cin >> type;
  std::cout << "width" << std::endl;
  uint32_t width = 0;
  std::cin >> width;
  if(access_type > 128 || index > 128 || type > 128 || width > 128) {
    return 0;
  }

  component_encoding encoding {
    .access_type = (uint8_t)access_type,
    .index = (uint8_t)index,
    .type = (uint8_t)type,
    .reserved1 = 0,
    .width = (uint8_t)width,
    .reserved2 = 0,
  };

  //std::cout << static_cast<uint32_t>(encoding) << std::endl;
  std::cout << (uint32_t)encoding.access_type << std::endl << (uint32_t)encoding.index << std::endl << (uint32_t)encoding.type << std::endl << (uint32_t)encoding.reserved1 << std::endl << (uint32_t)encoding.width << std::endl << (uint32_t)encoding.reserved2 << std::endl;
  

  std::cout << "number: " << static_cast<uint32_t>(encoding) << std::endl;
  return 0;
}*/
