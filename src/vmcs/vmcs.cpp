#include "vmcs.h"
#include "../pmm.h"
#include "../common.h"
#include "../mm/npaging.h"
#include "../fs/vfs.h"
#include "../pmm.h"
#include "../kheap.h"
#include "../instruction_decoder.h"
#include <cstdint>
#include <cstdio>
#include <math.h>
#include <bit>

#define QUANTOM 10

static uint32_t list_of_ports_to_intercept[] = {}; //{0x1f7};
static uint64_t vmcb_addr = 0;
static context guests[10] = {};
static uint64_t guests_count = 0;
static uint32_t curr_guest_idx = 0;
static uint32_t quantom_counter = 0;
static uint64_t ioio_map_addr = 0;
static uint64_t msrpm_base_addr = 0;

/* uint8_t detect_np_support() {
  uint64_t output = 0;
  uint64_t leaf = 0x8000000A;
  uint64_t sub_leaf = 0;
} */

void enable_svm() {
  // check if svm is supported
  uint32_t a, b, c, d;
  // asm volatile("cpuid" : "=d"(output) : "c"(leaf), "d"(sub_leaf));
  cpuid(0x80000001, a, b, c, d);
  if(getbit(c, 2) == 0) {
    asm volatile("hlt");
    return;
  }

  cpuid(0x8000000A, a, b, c, d);
  if(getbit(d, 0) == 0) {
    asm volatile("hlt");
    return;
  }

  uint64_t output = 0;
  // asm volatile("rdmsr" : "=a"(output) : "c"(0xC0000080));
  output = rdmsr(0xC0000080);
  setbit(&output, 12);

  // asm volatile("wrmsr" :: "c"(0xC0000080), "a"(output));
  wrmsr(0xC0000080, output);

  // asm volatile("rdmsr" : "=a"(output) : "c"(0xC0000080));
  output = rdmsr(0xC0000080);
  if(getbit(&output, 12) == 0) {
    asm volatile("hlt");
  }

  return;
}

void vmrun(uint64_t vmcb_addr) {
  asm volatile("mov %0, %%rax" :: "m"(vmcb_addr));
  // asm volatile("vmsave");
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

void init_vm() {
  // allocate memory for the vmcb
  uint64_t cr3_val = 0;
  asm volatile("mov %%cr3, %0" : "=r"(cr3_val));
  vmcb_addr = kpalloc();
  vmcb* vmcb_struct = (vmcb*)TO_HIGHER_HALF(vmcb_addr);
  kmemset((uint8_t*)TO_HIGHER_HALF(vmcb_addr), 0x0, sizeof(vmcb));

  uint64_t host_state_area = kpalloc();
  kmemset((uint8_t*)TO_HIGHER_HALF(host_state_area), 0x0, 0x1000);
  // asm volatile("wrmsr" :: "c"(0xC0010117), "a"(host_state_area & 0xFFFFFFFF), "d"(host_state_area >> 32));
  wrmsr(0xC0010117, host_state_area);

  // init ioio map
  ioio_map_addr = kpalloc_contignious(3);
  for(auto i : list_of_ports_to_intercept) {
    uint32_t byte = i / 8;
    uint32_t bit = i % 8;
    uint64_t* ioio_map_ptr = (uint64_t*)TO_HIGHER_HALF(ioio_map_addr);
    setbit(&ioio_map_ptr[byte], bit);
  }
  // check if this actually goes to the right place
  vmcb_struct->control.intercepts_insts_2 = (1 << 0);

  // init msr 
  msrpm_base_addr = kpalloc_contignious(2);
  kmemset((uint8_t*)TO_HIGHER_HALF(msrpm_base_addr), 0x0, 0x1000);
  
  vmcb_struct->control.intercept_exceptions = 0x0;
  // init vmcb struct with the right paramemeters
  vmcb_struct->state_save_area.cs.selector = 0xF000;
  vmcb_struct->state_save_area.cs.base = 0xFFFF0000;
  vmcb_struct->state_save_area.cs.limit = 0xFFFF;
  vmcb_struct->state_save_area.cs.attrib = 0x93;

  vmcb_struct->state_save_area.ds.selector = 0x0;
  vmcb_struct->state_save_area.ds.base = 0x0;
  vmcb_struct->state_save_area.ds.limit = 0xFFFF;
  vmcb_struct->state_save_area.ds.attrib = 0x93;

  vmcb_struct->state_save_area.es.selector = 0x0;
  vmcb_struct->state_save_area.es.base = 0x0;
  vmcb_struct->state_save_area.es.limit = 0xFFFF;
  vmcb_struct->state_save_area.es.attrib = 0x93;

  vmcb_struct->state_save_area.gs.selector = 0x0;
  vmcb_struct->state_save_area.gs.base = 0x0;
  vmcb_struct->state_save_area.gs.limit = 0xFFFF;
  vmcb_struct->state_save_area.gs.attrib = 0x93;

  vmcb_struct->state_save_area.fs.selector = 0x0;
  vmcb_struct->state_save_area.fs.base = 0x0;
  vmcb_struct->state_save_area.fs.limit = 0xFFFF;
  vmcb_struct->state_save_area.fs.attrib = 0x93;

  vmcb_struct->state_save_area.ss.selector = 0x0;
  vmcb_struct->state_save_area.ss.base = 0x0;
  vmcb_struct->state_save_area.ss.limit = 0xFFFF;
  vmcb_struct->state_save_area.ss.attrib = 0x93;

  vmcb_struct->state_save_area.gdtr.base = 0x0;
  vmcb_struct->state_save_area.gdtr.limit = 0xFFFF;
  vmcb_struct->state_save_area.gdtr.attrib = 0x0;
  vmcb_struct->state_save_area.gdtr.selector = 0x0;

  vmcb_struct->state_save_area.idtr.selector = 0x0;
  vmcb_struct->state_save_area.idtr.base = 0x0;
  vmcb_struct->state_save_area.idtr.limit = 0x1FFF;
  vmcb_struct->state_save_area.idtr.attrib = 0x0;

  vmcb_struct->state_save_area.ldtr.selector = 0x0;
  vmcb_struct->state_save_area.ldtr.base = 0x0;
  vmcb_struct->state_save_area.ldtr.limit = 0xFFFF;
  vmcb_struct->state_save_area.ldtr.attrib = 0x82;

  vmcb_struct->state_save_area.tr.selector = 0x0;
  vmcb_struct->state_save_area.tr.base = 0x0;
  vmcb_struct->state_save_area.tr.limit = 0xFFFF;
  vmcb_struct->state_save_area.tr.attrib = 0x83;

  vmcb_struct->state_save_area.rip = 0x8000;
  vmcb_struct->state_save_area.cr0 = (1 << 29) | (1 << 30); // | (1 << 4);
  vmcb_struct->state_save_area.cr2 = 0x0;
  vmcb_struct->state_save_area.cr3 = 0x0;
  vmcb_struct->state_save_area.cr4 = 0x0;
  vmcb_struct->state_save_area.rflags = 1 << 1;
  vmcb_struct->state_save_area.efer = (1 << 12);
  vmcb_struct->state_save_area.rax = 0x0;
  vmcb_struct->control.interrupt_shadow = 0x0;
  vmcb_struct->state_save_area.rsp = 0x0;
  vmcb_struct->state_save_area.dr6 = 0xFFFF0FF0;
  vmcb_struct->state_save_area.dr7 = 0x400;
  vmcb_struct->state_save_area.rsp = 0x0;

  // vmcb_struct->control.nrip = 0xFFF0;
  vmcb_struct->control.np_enable = 1;

  vmcb_struct->control.guest_asid = 4;
  vmcb_struct->control.iopm_base_pa = ioio_map_addr;
  vmcb_struct->control.msrpm_base_pa = msrpm_base_addr;
  vmcb_struct->control.intercept_cr_reads = 1;
  vmcb_struct->control.intercepts_cr_writes = 1;

  vmcb_struct->control.intercept_exceptions = (1 << 1) | (1 << 6) | (1 << 14) | (1 << 17) | (1 << 18);
  vmcb_struct->control.intercept_exceptions &= ~((1 << 14) | (1 << 6));
  vmcb_struct->control.tlb_control = 0x0;
  vmcb_struct->control.vmcb_clean_bits = 0;

  // load coreboot to memory address starting with RIP
  uint32_t fd = vopenFile((char*)"vm_test.bin"); 
  //map to rip address
  uint32_t file_size = vgetFileSize(fd);
  uint32_t num_of_pages = (file_size + 0x1000 - 1) / 0x1000;
  uint64_t vm_mem_map = create_clean_virtual_space();
  
  uint64_t boot_base_addr = vmcb_struct->state_save_area.cs.base + vmcb_struct->state_save_area.rip;
  for(uint32_t i = 0; i < num_of_pages; i++) {
    uint64_t coreboot_page = kpalloc();
    mapPage(coreboot_page + i * 0x1000,
             boot_base_addr + i * 0x1000,
             0x7,
             vm_mem_map);

    vreadFile(fd, (char*)(TO_HIGHER_HALF(coreboot_page)), 0x1000);
    // vmcb_struct->state_save_area.rip = coreboot_page;
  }

  // identity_map(vm_mem_map, 0x0, 0x1000, 0x7);
   
  vmcb_struct->control.n_cr3 = vm_mem_map;

  // create ide virtual driver
  // virtual_storage_device* storage_dev = new virtual_storage_device((char*)"ide_disk", 512);
  // ata_pio_device* ata_device = new ata_pio_device(storage_dev); 
  // create virtual cmos 
  // vmrun
  
  enable_svm();
  vmrun(vmcb_addr);           // MUST DEBUG WITH STEPI!!!!!!
}

void vmexit_handler() {
  vmcb* vmcb_struct = (vmcb*)vmcb_addr;
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
  // @TODO: add handling to REP prefix
  vmcb* vmcb_struct = (vmcb*)vmcb_addr;
  uint64_t exitinfo1_val = vmcb_struct->control.exitinfo1;
  ioio_exitinfo1* exitinfo1 = std::bit_cast<ioio_exitinfo1*>(&exitinfo1_val);

  if(((exitinfo1->port >= IO_MASTER_BASE_PORT) && (exitinfo1->port <= (IO_MASTER_BASE_PORT + 7)))      ||
    ((exitinfo1->port >= IO_MASTER_BASE_CONTROL) && (exitinfo1->port <= (IO_MASTER_BASE_CONTROL + 1))) ||
    ((exitinfo1->port >= IO_SLAVE_BASE_PORT) && (exitinfo1->port <= (IO_SLAVE_BASE_PORT + 7)))         ||
    ((exitinfo1->port >= IO_SLAVE_CONTROL) && (exitinfo1->port <= (IO_SLAVE_CONTROL + 1)))) {
      //uint32_t port = exitinfo1->port;
      //operands_decoding inst_decode = decode_in(vmcb_struct->state_save_area.rip);
      ide_transaction transaction{.exitinfo = *exitinfo1, .written_val = 0};
      if(exitinfo1->type == 1) {
        transaction.written_val = vmcb_struct->state_save_area.rax;
      }

      //uint64_t output = guests[curr_guest_idx].ide.handle_transaction(transaction);
      guests[curr_guest_idx].ata_device->dispatch_command(transaction);
  }
}

void inject_event(uint8_t vector, event_type_e type, uint8_t push_error_code, uint32_t error_code) {
  if(push_error_code > 1) {
    return;
  }
  
  vmcb* vmcb_struct = (vmcb*)vmcb_addr;
  vmcb_struct->control.eventinj.vector = vector;
  vmcb_struct->control.eventinj.type = type;
  vmcb_struct->control.eventinj.error_code_valid = push_error_code;
  vmcb_struct->control.eventinj.errorcode = error_code;
}

void edit_vmcb_state(vmcb_registers reg, uint64_t value) {
  vmcb* vmcb_struct = (vmcb*)vmcb_addr;
  switch (reg) {
    case RAX:
      vmcb_struct->state_save_area.rax = value;

    default:
      break;
  }
}
