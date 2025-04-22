#include "scheduler.h"
#include "task.h"
#include <cstdint>
#include "../mm/npaging.h"
#include "../mm/paging.h"
#include "../gdt.h"
#include "../elf/elfloader.h"

#define MAX_NUMBER_OF_TASKS 100
#define QUANTOM 10

static uint32_t quantom_counter = 0;
static task_context_s tasks_list[100] = {0};
static uint32_t last_task_idx = 0;
static uint32_t curr_task_idx = 1;
static bool scheduler_initialized = false;

static uint32_t find_next_task(uint32_t curr_task);
static void idle_task();

void init_scheduler() {
  create_kernel_task((uint64_t)idle_task);
  scheduler_initialized = true;
}

void round_robin(Stack* stack) {
  if(!scheduler_initialized) {
    //write_to_port((uint16_t)0x20, (uint8_t)0x20);
    return;
  }

  quantom_counter++;
  if(quantom_counter < QUANTOM) {
    //write_to_port((uint16_t)0x20, (uint8_t)0x20);
    return;
  }

  quantom_counter = 0;
  uint32_t next_task_idx = find_next_task(curr_task_idx);
  //write_to_port((uint16_t)0x20, (uint8_t)0x20);
  switch_context((uint64_t)stack, next_task_idx);
}

void switch_context(uint64_t stack_addr, uint32_t next_task_idx) {
  kmemcpy(
      (uint8_t*)&tasks_list[curr_task_idx].stack,
      (uint8_t*)stack_addr,
      sizeof(Stack)
  );

  kmemcpy(
      (uint8_t*)stack_addr,
      (uint8_t*)&tasks_list[next_task_idx].stack,
      sizeof(Stack)
  );
  
  // if(tasks_list[curr_task_idx].userland != tasks_list[next_task_idx].userland) {
  switch_pageMap(tasks_list[next_task_idx].mem_map);
  //}

  curr_task_idx = next_task_idx;
}

// 1. load the ELF into memory
// 2. create a new memory mapping, including the kernel.
// 3. create a new task object, create a stack space for the process and according to the dpl sysret into it or just switch the current stack
TASK_ID create_kernel_task(uint64_t start_rip) {
  uint32_t new_task_idx = last_task_idx;
  kmemset((uint8_t*)&tasks_list[last_task_idx], 0x0, sizeof(task_context_s));
  tasks_list[last_task_idx].stack.cs = CODE64_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rsp = create_stack_frame(get_host_pageMap(), false);
  tasks_list[last_task_idx].stack.rip = start_rip;
  tasks_list[last_task_idx].mem_map = get_host_pageMap();
  tasks_list[last_task_idx].userland = false;
  tasks_list[last_task_idx].stack.ss = DATA64_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rflags = (1 << 1) | (1 << 9); // IF FLAG AND RESERVED BIT(1)
  last_task_idx++;
  return new_task_idx;
}

TASK_ID create_user_task(uint32_t elf_fd) {
  uint32_t next_task_idx = last_task_idx;
  kmemset((uint8_t*)&tasks_list[last_task_idx], 0x0, sizeof(task_context_s));
  uint64_t user_memmap = create_clean_virtual_space();
  init_mappings(user_memmap);
  uint64_t start_rip = load_elf_to_memory(elf_fd, user_memmap);

  tasks_list[last_task_idx].stack.cs = CODE64_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rsp = create_stack_frame(user_memmap, true);
  tasks_list[last_task_idx].stack.rip = start_rip;

  tasks_list[last_task_idx].mem_map = user_memmap;
  tasks_list[last_task_idx].userland = false;
  tasks_list[last_task_idx].stack.ss = DATA64_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rflags = (1 << 1) | (1 << 9); // IF FLAG AND RESERVED BIT(1)
  last_task_idx++;
  return next_task_idx;
}

void kill_task(TASK_ID task_id) {

}

uint64_t create_stack_frame(uint64_t mem_map, bool is_user) {
  uint64_t kernel_pagemap = switch_host_pagemap(mem_map);
  uint64_t phys_addr = 0;
  uint64_t virt_addr = 0;
  if(is_user) {
    allocate_page(&virt_addr, &phys_addr, 0x07);
  } else {
    allocate_page(&virt_addr, &phys_addr, 0x03);
  }

  return virt_addr;
}

uint32_t find_next_task(uint32_t curr_task) {
  for(uint32_t next_task = curr_task + 1; next_task < MAX_NUMBER_OF_TASKS; next_task++) {
    if(tasks_list[next_task].alive) {
      return next_task;
    }
  }

  for(uint32_t next_task = 1; next_task < curr_task; next_task++) {
    if(tasks_list[next_task].alive) {
      return next_task;
    }
  }

  return 0;
}

void idle_task() {
  while(1);
}
