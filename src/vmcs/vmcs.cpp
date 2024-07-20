#include "vmcs.h"
#include <cstdint>

void vmwrite(VMCS_FIELD field, uint64_t data) {
  asm volatile("vmwrite %0, %1" :: "r"((uint64_t)field), "r"(data));
}

uint64_t vmread(VMCS_FIELD field) {
  uint64_t output = 0;
  asm volatile("vmread %0, %1" : "=r"(output) : "r"((uint64_t)field));
  return output;
}

void vmptrld(uint64_t vmcs_ptr) {
 asm volatile("vmptrld %0" :: "m"(vmcs_ptr));
}
