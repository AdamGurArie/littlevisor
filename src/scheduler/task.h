#pragma once

#include "../common.h"
#include <cstdint>

struct task_context_s {
  Stack stack;
  uint64_t mem_map;
  bool userland;
  bool alive;
};
