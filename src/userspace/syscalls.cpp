#include "syscalls.h"
#include "../mm/paging.h"
#include "../gdt.h"
#include <cstdint>
#include "../common.h"
#include "../scheduler/scheduler.h"
#include "fs_syscalls.h"
#include "term_syscalls.h"
#include "../mm/npaging.h"
#include "../timers/pit.h"

#define LSTAR_MSR 0xC0000082
#define STAR_MSR 0xC0000081

extern "C" void jump_to_userland_asm(uint64_t rip, uint64_t rsp, uint64_t rflags, uint64_t ret_val);
extern "C" void syscall_tramp();

uint8_t* syscall_handler_stack = 0;
uint64_t host_pagemap = 0;

void init_syscalls() {
  uint64_t phys_addr;
  allocate_page(
      (uint64_t*)&syscall_handler_stack,
      &phys_addr,
      0x03, // @TODO: refactor
      0
  );
  
  //uint64_t star_msr_val = rdmsr(STAR_MSR);// | ((uint64_t)DATA64_DESC_SELECTOR << 48);
  
  //wrmsr(STAR_MSR, star_msr_val);
  wrmsr(LSTAR_MSR, (uint64_t)syscall_tramp);

  switch_tss_kernelstack((uint64_t)syscall_handler_stack + 0x1000);
  host_pagemap = get_host_pageMap();
}

uint64_t dispatch_syscall(uint64_t vector, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
  
  // uint64_t user_memmap = switch_to_host_pagemap();
  uint64_t rsp = 0;
  asm volatile ("mov %0, %%r12" : "=r"(rsp));

  uint64_t ret_val = 0;
  switch(vector) {
    case FS_OPEN_FILE:
      ret_val = open_file_syscall((char*)arg1);
      break;
    
    case FS_READ_FILE:
      read_file_syscall((uint32_t)arg1, (uint8_t*)arg2, (uint32_t)arg2);
      break;

    case FS_WRITE_FILE:
      write_file_syscall((uint32_t)arg1, (uint8_t*)arg2, (uint32_t)arg3);
      break;

    case FS_SEEK_FILE:
      seek_file_syscall((uint32_t)arg1, (uint32_t)arg2);
      break;

    case PUTS:
      puts_syscall((const char*)arg1);

    case SLEEP:
      sleep((uint64_t)arg1);
    
    default:
      break;
  }
  
  // switch_pageMap(user_memmap);
  return ret_val;
}

void jump_to_userland(uint64_t rsp, uint64_t rip, uint64_t rflags, uint64_t ret_val) {
  switch_tss_userstack(rsp);
  jump_to_userland_asm(rip, rsp, rflags, ret_val);
}
