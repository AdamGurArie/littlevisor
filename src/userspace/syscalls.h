#pragma once

#include <cstdint>

enum syscall_vectors_e {
  FS_OPEN_FILE,
  FS_READ_FILE,
  FS_WRITE_FILE,
  FS_SEEK_FILE,
  PUTS,
  SLEEP
};

void init_syscalls();
uint64_t dispatch_syscall(uint64_t vector, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void jump_to_userland(uint64_t rsp, uint64_t rip, uint64_t rflags, uint64_t ret_val);
