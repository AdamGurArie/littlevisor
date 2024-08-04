#include "vmcs.h"
#include "../pmm.h"
#include "../common.h"
#include "../mm/npaging.h"
#include "../drivers/ide.h"
#include "../instruction_decoder.h"
#include <cstdint>
#include <bit>

#define QUANTOM 10

static uint64_t vmcb_addr = 0;
static context guests[10] = {};
static uint64_t guests_count = 0;
static uint32_t curr_guest_idx = 0;
static uint32_t quantom_counter = 0;
static uint64_t ioio_map_addr = 0;

void vmrun(uint64_t vmcb_addr) {
  asm volatile("mov %0, %%rax" :: "m"(vmcb_addr));
  asm volatile("vmrun");
  // 1. handle vm-exit
  // 2. scheduale next vm
}

void add_guest(uint64_t entry_point, segment_selector cs) {
  guests[guests_count].guest = vmcb_state_save_area {};
  guests[guests_count].guest.cs = cs;
  guests[guests_count].guest.rip = entry_point;
  guests[guests_count].guest.rflags = 0;
  guests[guests_count].guest.rax = 0;
  guests[guests_count].guest.ss = segment_selector{.selector = 0, .attrib = 0, .limit = 0, .base = 0 }; 
  guests[guests_count].guest.rsp = 0;
  guests[guests_count].guest.cr0 = 0;
  guests[guests_count].guest.cr2 = 0;
  guests[guests_count].guest.cr3 = 0;
  guests[guests_count].guest.cr4 = 0;
  guests[guests_count].guest.efer = 0;
  guests[guests_count].guest.idtr = segment_selector{.selector = 0, .attrib = 0, .limit = 0, .base = 0 };
  guests[guests_count].guest.gdtr = segment_selector{.selector = 0, .attrib = 0, .limit = 0, .base = 0 }; 
  guests[guests_count].guest.es = segment_selector{.selector = 0, .attrib = 0, .limit = 0, .base = 0 }; 
  guests[guests_count].guest.ds = segment_selector{.selector = 0, .attrib = 0, .limit = 0, .base = 0 }; 
  guests[guests_count].guest.dr6 = 0;
  guests[guests_count].guest.dr7 = 0;
  guests[guests_count].guest.cpl = 0;

  uint64_t new_pagedir = kpalloc();
  save_host_pageMap();
  switch_pageMap(new_pagedir);
  create_linear_virtual_space(MEMORY_SPACE_PER_VM);
}

void scheduale() {
  quantom_counter++;
  if(quantom_counter >= 10) {
    kmemcpy((uint8_t*)&guests[curr_guest_idx], (uint8_t*)(vmcb_addr + 0x3FF), sizeof(vmcb_state_save_area));
    curr_guest_idx = curr_guest_idx >= guests_count ? 0 : curr_guest_idx+1;
    kmemcpy((uint8_t*)(vmcb_addr + 0x3FF), (uint8_t*)&guests[curr_guest_idx], sizeof(vmcb_state_save_area));
  }
}

void vmrun() {
  asm volatile("mov %%rax, %0" :: "r"(vmcb_addr));
  asm volatile("vmrun");
}

void init_vm() {
  vmcb_addr = kpalloc();
  ioio_map_addr = kpalloc_contignious(3);
  for(auto i : list_of_ports_to_intercept) {
    uint32_t byte = i / 8;
    uint32_t bit = i % 8;
    uint64_t* ioio_map_ptr = (uint64_t*)ioio_map_addr;
    setbit(&ioio_map_ptr[byte], bit);
  }

  vmrun();
}

void vmexit_handler() {
  vmcb* vmcb_struct = std::bit_cast<vmcb*>(vmcb_addr);
  switch(vmcb_struct->control.exitcode) {
    case VMEXIT_IOIO : {
      handle_ioio_vmexit(); 
    }
  }
}

void handle_ioio_vmexit() {
  // determine the correct handler 
  // call the handler function
  // get the return value
  // return it to the destination register/address(if needed)
  vmcb* vmcb_struct = std::bit_cast<vmcb*<(vmcb_addr);
  ioio_exitinfo1* exitinfo1 = std::bit_cast<ioio_exitinfo1*>(&vmcb_struct->control.exitinfo1);
  if(((exitinfo1->port >= IO_MASTER_BASE_PORT) && (exitinfo1->port <= (IO_MASTER_BASE_PORT + 7)))      ||
    ((exitinfo1->port >= IO_MASTER_BASE_CONTROL) && (exitinfo1->port <= (IO_MASTER_BASE_CONTROL + 1))) ||
    ((exitinfo1->port >= IO_SLAVE_BASE_PORT) && (exitinfo1->port <= (IO_SLAVE_BASE_PORT + 7)))         ||
    ((exitinfo1->port >= IO_SLAVE_CONTROL) && (exitinfo1->port <= (IO_SLAVE_CONTROL + 1)))) {
      uint32_t port = exitinfo1->port;
      //operands_decoding inst_decode = decode_in(vmcb_struct->state_save_area.rip);
      ide_transaction transaction{.exitinfo = *exitinfo1, .written_val = 0};
      if(exitinfo1->type == 1) {
        transaction.written_val = vmcb_struct->state_save_area.rax;
      }

      uint64_t output = guests[curr_guest_idx].ide.handle_transaction(transaction);
      if(exitinfo1->type == 0) {
        vmcb_struct->state_save_area.rax = output;
      }
  }
}
