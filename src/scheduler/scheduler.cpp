#include "scheduler.h"
#include "task.h"
#include <cstdint>
#include "../mm/npaging.h"
#include "../mm/paging.h"
#include "../gdt.h"
#include "../elf/elfloader.h"
#include "../timers/pit.h"

#define MAX_NUMBER_OF_TASKS 100
#define QUANTOM 10

#define STAR_MSR 0xC0000081
#define LSTAR_MSR 0xC0000082
#define IA32_EFER 0xC0000080

static uint32_t quantom_counter = 0;
static task_context_s tasks_list[100] = {0};
static uint32_t last_task_idx = 2;
static uint32_t curr_task_idx = 1;
static uint32_t prev_task_idx = 1;
static bool scheduler_initialized = false;
static uint64_t kernel_stack = 0;

static uint32_t find_next_task(uint32_t curr_task);
static void idle_task();

extern "C" void jump_to_userland_asm(uint64_t rip, uint64_t rsp, uint64_t rflags);

void enable_syscalls() {
  wrmsr(IA32_EFER, rdmsr(IA32_EFER) | (1 << 0));
  uint64_t star_value = (uint64_t)(CODE64_USER_DESC_SELECTOR - 16) << 48; // This is only right for INTEL X86. For AMD X86, the offset is 50 bits into STAR MSR, read the AMD manual.
  wrmsr(STAR_MSR, star_value);
  star_value = rdmsr(STAR_MSR);
}

void init_scheduler() {
  kmemset((uint8_t*)&tasks_list[0], 0x0, sizeof(task_context_s));
  tasks_list[0].stack.cs = CODE64_DESC_SELECTOR;
  tasks_list[0].stack.rsp = create_stack_frame(get_host_pageMap(), false);
  tasks_list[0].stack.rip = (uint64_t)idle_task;
  tasks_list[0].mem_map = get_host_pageMap();
  tasks_list[0].userland = false;
  tasks_list[0].stack.ss = DATA64_DESC_SELECTOR;
  tasks_list[0].stack.rflags = (1 << 1) | (1 << 9); // IF FLAG AND RESERVED BIT(1)
  tasks_list[0].alive = true;

  enable_syscalls();
  uint64_t kernel_phys_stack = 0;
  allocate_page(&kernel_stack, &kernel_phys_stack, 0x03, 0);
  scheduler_initialized = true;
}

void decrement_delays() {
  for (uint32_t i = 0; i < last_task_idx; i++) {
    if(tasks_list[i].alive && tasks_list[i].delay_ticks > 0) {
      tasks_list[i].delay_ticks -= 1;
    }
  }
}

void round_robin(Stack* stack) {
  if(!scheduler_initialized) {
    //write_to_port((uint16_t)0x20, (uint8_t)0x20);
    return;
  }

  quantom_counter++;
  decrement_delays(); // @TODO: but decrement should happen 1ms and round robin is called every tick, fix it.
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
      (uint8_t*)&tasks_list[prev_task_idx].stack,
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
  asm volatile("invlpg (%0)" :: "r"(tasks_list[next_task_idx].mem_map) : "memory");
  //}
  
  prev_task_idx = curr_task_idx;
  curr_task_idx = next_task_idx;
  if(tasks_list[next_task_idx].userland) {
    switch_tss_userstack(tasks_list[next_task_idx].stack.rsp);
    switch_tss_kernelstack(kernel_stack + 0x1000);
    jump_to_userland(next_task_idx);
  }
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
  tasks_list[last_task_idx].alive = true;
  last_task_idx++;
  return new_task_idx;
}

TASK_ID create_user_task(uint32_t elf_fd) {
  uint32_t next_task_idx = last_task_idx;
  kmemset((uint8_t*)&tasks_list[last_task_idx], 0x0, sizeof(task_context_s));
  uint64_t user_memmap = create_clean_virtual_space();
  init_user_mappings(user_memmap);
  uint64_t start_rip = load_elf_to_memory(elf_fd, user_memmap);
  if(start_rip == 0x0) {
    return 0x0; // failed to create task
  }

  tasks_list[last_task_idx].stack.cs = CODE64_USER_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rsp = create_stack_frame(user_memmap, true);
  tasks_list[last_task_idx].stack.rip = start_rip;

  tasks_list[last_task_idx].mem_map = user_memmap;
  tasks_list[last_task_idx].userland = true;
  tasks_list[last_task_idx].stack.ss = DATA64_USER_DESC_SELECTOR;
  tasks_list[last_task_idx].stack.rflags = (1 << 1) | (1 << 9); // IF FLAG AND RESERVED BIT(1)
  tasks_list[last_task_idx].alive = true;
  last_task_idx++;
  return next_task_idx;
}

void kill_task(TASK_ID task_id) {

}

uint64_t create_stack_frame(uint64_t mem_map, bool is_user) {
  uint64_t phys_addr = 0;
  uint64_t virt_addr = 0;
  if(is_user) {
    allocate_page(&virt_addr, &phys_addr, 0x07, mem_map);
  } else {
    allocate_page(&virt_addr, &phys_addr, 0x03, mem_map);
  }

  return (virt_addr + PAGE_SIZE);
}

uint32_t find_next_task(uint32_t curr_task) {
  for(uint32_t next_task = curr_task + 1; next_task < MAX_NUMBER_OF_TASKS; next_task++) {
    if(tasks_list[next_task].alive && tasks_list[next_task].delay_ticks == 0) {
      return next_task;
    }
  }

  for(uint32_t next_task = 1; next_task < curr_task; next_task++) {
    if(tasks_list[next_task].alive && tasks_list[next_task].delay_ticks == 0) {
      return next_task;
    }
  }

  return 0;
}

void jump_to_userland(uint64_t next_task_idx) {
  switch_tss_userstack(tasks_list[next_task_idx].stack.rsp);
  jump_to_userland_asm(tasks_list[next_task_idx].stack.rip, tasks_list[next_task_idx].stack.rsp, tasks_list[next_task_idx].stack.rflags);
}

void delay_task(uint64_t ms, uint64_t stack) {
  tasks_list[curr_task_idx].delay_ticks = ms / get_ms_per_tick();
  quantom_counter = QUANTOM;
  round_robin((Stack*)stack);
}

void idle_task() {
  while(1);
}
