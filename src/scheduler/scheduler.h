#pragma once

#include <cstdint>
#include "task.h"

typedef uint8_t TASK_ID;

void init_scheduler();
void switch_context(uint64_t stack, uint32_t next_task_idx);
TASK_ID create_kernel_task(uint64_t start_rip);
TASK_ID create_user_task(uint32_t elf_fd);
void kill_task(TASK_ID task_id);
uint64_t create_stack_frame(uint64_t mem_map, bool is_user);
void round_robin(Stack* stack);
void jump_to_userland(uint64_t next_task_idx);
void delay_task(uint64_t ms, uint64_t stack);
