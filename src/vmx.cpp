#include "vmx.h"
#include "common.h"

#include <cstdint>

bool enable_vmx() {
  uint32_t eax_val = 0;
  asm volatile("mov $0, %%eax" ::: "eax");
  asm volatile("cpuid");
  asm volatile("mov %%ecx, %0" : "=r"(eax_val));
  if(getbit((uint64_t)eax_val, 5) == 0) {
    return false;
  }
  
  uint64_t cr4_val = 0;
  asm volatile("mov %%cr4, %0" : "=r"(cr4_val));
  cr4_val = setbit(cr4_val, 13); 
  asm volatile("mov %0, %%cr4" :: "r"(cr4_val));
  asm volatile("vmxon");
  return true;
}
