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
#include <string>
#include <algorithm>


// @TODO: try to load seabios into the host's memory so I'll be able to see the instructions there.
// Then, possibly I'll be able to figure out why it gets into an infinite loop

#define QUANTOM 10
#define INSTRUCTION_VMRUN_INTERCEPT (1 << 0)

#define CR0_NOT_WRITE_THROUGH (1 << 29)
#define CR0_CACHE_DISABLE (1 << 30)

#define RIP_INIT_VALUE 0xFFF0

#define RFLAGS_DEFAULT_INIT_VAL (1 << 1)

#define EFER_SVM_ENABLE (1 << 12)

#define INTERCEPT_EXCEPTION_NMI (1 << 3)
#define INTERCEPT_EXCEPTION_BP (1 << 4)
#define INTERCEPT_EXCEPTION_PF (1 << 15)

extern "C" void svm_vmrun(guest_regs* regs, uint64_t vmcb);

static uint64_t calculate_physical_addr(uint16_t seg, uint16_t off);
static uint64_t calculate_physical_addr(uint32_t seg, uint32_t off);

//static uint32_t list_of_ports_to_intercept[] = {0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x3f6, 0x3f7, 0x514, 0x518, 0x510, 0x511};
static uint32_t list_of_ports_to_intercept[] = {0x514, 0x518, 0x510, 0x511, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f6, 0x1f7, 0x60608, 0x4240, 0x4241, 0x4242, 0x4243, 0x608};
//static uint32_t list_of_ports_to_intercept[] = {0x514, 0x518, 0x510, 0x511, 0x1f3};
static uint64_t vmcb_addr = 0;
static uint64_t host_state_area = 0;
static context guests[10] = {};
static uint64_t guests_count = 0;
static uint32_t curr_guest_idx = 0;
static uint32_t quantom_counter = 0;
static uint64_t ioio_map_addr = 0;
static uint64_t msrpm_base_addr = 0;
static bool test = false;

/* uint8_t detect_np_support() {
  uint64_t output = 0;
  uint64_t leaf = 0x8000000A;
  uint64_t sub_leaf = 0;
} */

static uint64_t calculate_physical_addr(uint16_t seg, uint16_t off) {
  uint64_t effective_addr = (seg * 0x10) + off;
  return walkTable(effective_addr, guests[curr_guest_idx].guest_cr3);
}

static uint64_t calculate_physical_addr(uint32_t seg, uint32_t off) {
  uint64_t effective_addr = seg + off;
  return walkTable(effective_addr, guests[curr_guest_idx].guest_cr3);
}

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

void vmsave(uint64_t host_state_area_addr) {
  asm volatile("mov %0, %%rax" :: "m"(host_state_area_addr));
  asm volatile("vmsave");
}

void vmload(uint64_t host_state_area_addr) {
  asm volatile("mov %0, %%rax" :: "m"(host_state_area));
  asm volatile("vmload");
}

void vmrun(uint64_t vmcb) {
  asm volatile("push %rbp");
  asm volatile("push %rbx");
  asm volatile("push %r12");
  asm volatile("push %r13");
  asm volatile("push %r14");
  asm volatile("push %r15");
  asm volatile("push %rsi");

  asm volatile("mov %0, %%rax" :: "m"(vmcb));
  asm volatile("vmrun");

  asm volatile("pop %r15");
  asm volatile("pop %r14");
  asm volatile("pop %r13");
  asm volatile("pop %r12");
  asm volatile("pop %rbx");
  asm volatile("pop %rbp");
  // 1. handle vm-exit
  // 2. scheduale next vm
}

void scheduale() {
  quantom_counter++;
  if(quantom_counter >= 10) {
    kmemcpy((uint8_t*)&guests[curr_guest_idx], (uint8_t*)(vmcb_addr + 0x3FF), sizeof(vmcb_state_save_area));
    curr_guest_idx = curr_guest_idx >= guests_count ? 0 : curr_guest_idx+1;
    kmemcpy((uint8_t*)(vmcb_addr + 0x3FF), (uint8_t*)&guests[curr_guest_idx], sizeof(vmcb_state_save_area));
  }
}

void context_switching(uint16_t curr_guest, uint16_t next_guest) {
  if(curr_guest >= 100 || next_guest >= 100) {
    return;
  }

  vmcb* vmcb_struct = (vmcb*)TO_HIGHER_HALF(vmcb_addr);
  kmemcpy(
      (uint8_t*)&guests[curr_guest].guest,
      (uint8_t*)&vmcb_struct->state_save_area,
      sizeof(vmcb_struct->state_save_area)
  );

  guests[curr_guest].guest_cr3 = vmcb_struct->control.n_cr3;
  guests[curr_guest].guest_asid = vmcb_struct->control.guest_asid;

  kmemcpy(
      (uint8_t*)&vmcb_struct->state_save_area,
      (uint8_t*)&guests[next_guest].guest,
      sizeof(vmcb_struct->state_save_area)
  );

  vmcb_struct->control.n_cr3 = guests[next_guest].guest_cr3;
  vmcb_struct->control.guest_asid = guests[next_guest].guest_asid;
  
  curr_guest_idx = next_guest;
  // vmrun(vmcb_addr);
  svm_vmrun(&guests[curr_guest_idx].regs, vmcb_addr);
}

void init_guest_state(uint16_t guest_idx, const char* codefile) {
  guests[guest_idx].guest.cs.selector = 0xF000;
  guests[guest_idx].guest.cs.base = 0xFFFF0000;
  guests[guest_idx].guest.cs.limit = 0xFFFF;
  guests[guest_idx].guest.cs.attrib = 0x93;

  guests[guest_idx].guest.ds.selector = 0x0;
  guests[guest_idx].guest.ds.base = 0x0;
  guests[guest_idx].guest.ds.limit = 0xFFFF;
  guests[guest_idx].guest.ds.attrib = 0x93;

  guests[guest_idx].guest.es.selector = 0x0;
  guests[guest_idx].guest.es.base = 0x0;
  guests[guest_idx].guest.es.limit = 0x0;
  guests[guest_idx].guest.es.attrib = 0x93;

  guests[guest_idx].guest.gs.selector = 0x0;
  guests[guest_idx].guest.gs.base = 0x0;
  guests[guest_idx].guest.gs.limit = 0xFFFF;
  guests[guest_idx].guest.gs.attrib = 0x93;

  guests[guest_idx].guest.fs.selector = 0x0;
  guests[guest_idx].guest.fs.base = 0x0;
  guests[guest_idx].guest.fs.limit = 0xFFFF;
  guests[guest_idx].guest.fs.attrib = 0x93;

  guests[guest_idx].guest.ss.selector = 0x0;
  guests[guest_idx].guest.ss.base = 0x0;
  guests[guest_idx].guest.ss.limit = 0xFFFF;
  guests[guest_idx].guest.ss.attrib = 0x93;

  guests[guest_idx].guest.gdtr.selector = 0x0;
  guests[guest_idx].guest.gdtr.base = 0x0;
  guests[guest_idx].guest.gdtr.limit = 0xFFFF;
  guests[guest_idx].guest.gdtr.attrib = 0x0;

  guests[guest_idx].guest.idtr.selector = 0x0;
  guests[guest_idx].guest.idtr.base = 0x0;
  guests[guest_idx].guest.idtr.limit = 0x1FFF;
  guests[guest_idx].guest.idtr.attrib = 0x0;

  guests[guest_idx].guest.ldtr.selector = 0x0;
  guests[guest_idx].guest.ldtr.base = 0x0;
  guests[guest_idx].guest.ldtr.limit = 0xFFFF;
  guests[guest_idx].guest.ldtr.attrib = 0x82;

  guests[guest_idx].guest.tr.selector = 0x0;
  guests[guest_idx].guest.tr.base = 0x0;
  guests[guest_idx].guest.tr.limit = 0xFFFF;
  guests[guest_idx].guest.tr.attrib = 0x83;

  guests[guest_idx].guest.rip = RIP_INIT_VALUE;
  guests[guest_idx].guest.cr0 = CR0_CACHE_DISABLE | CR0_NOT_WRITE_THROUGH;
  guests[guest_idx].guest.rflags = RFLAGS_DEFAULT_INIT_VAL;
  guests[guest_idx].guest.efer = EFER_SVM_ENABLE;

  guests[guest_idx].guest.dr6 = 0xFFFF0FF0;
  guests[guest_idx].guest.dr7 = 0x400;

  uint64_t vm_mem_map = create_clean_virtual_space();
  guests[guest_idx].guest_cr3 = vm_mem_map;
  guests[guest_idx].guest_asid = guest_idx;

  storage_device* storage_dev = new virtual_storage_device(const_cast<const char*>("storage"), 512);
  guests[guest_idx].ata_device = new ata_pio_device(storage_dev);
  
  uint32_t file_handle = vopenFile(codefile);
  if(file_handle == OPEN_FILE_ERROR) {
    kpanic();
  }

  uint64_t file_size = vgetFileSize(file_handle); 
  uint32_t num_of_pages = (file_size + PAGE_SIZE - 1) / PAGE_SIZE;
  uint64_t bootloader_high_addr = 0x100000000;
  uint64_t isa_bootloader_high_addr = 0x100000;
  uint64_t isa_bootloader_start = kmin(128*1024, file_size);
  uint64_t himem_size = 128 * 1024 * 1024; 
  // uint64_t isa_bootloader_start_addr = isa_bootloader_high_addr - num_of_pages * PAGE_SIZE;
  uint64_t isa_bootloader_start_addr = isa_bootloader_high_addr - num_of_pages * PAGE_SIZE;
  uint64_t size_read = 0;
  uint64_t isa_curr = isa_bootloader_start;
  for(int64_t i = num_of_pages; i > 0; i--) {
    uint64_t phys_page = kpalloc();
    if(phys_page == 0) {
      kpanic();
    }

    mapPage(
        phys_page,
        bootloader_high_addr - i * PAGE_SIZE,
        GUEST_PHYSICAL_PAGE_FLAG,
        vm_mem_map
    );
    

    /*if((file_size - i * PAGE_SIZE) <= isa_bootloader_start) {
      mapPage(
          phys_page,
          isa_bootloader_start + isa_curr, 
          GUEST_PHYSICAL_PAGE_FLAG,
          vm_mem_map
      );

      isa_curr -= PAGE_SIZE;
    }*/

    uint32_t size_to_read = (file_size - size_read) > PAGE_SIZE ? PAGE_SIZE : (file_size - size_read);
    vreadFile(
        file_handle,
        (char*)TO_HIGHER_HALF(phys_page),
        size_to_read
    );

    vseekr(file_handle, PAGE_SIZE);
    size_read += size_to_read;
  }

  for(uint64_t i = 0; i < (bootloader_high_addr - num_of_pages * PAGE_SIZE); i += 0x1000) {
    uint64_t page = kpalloc();
    if(page == 0) {
      kpanic();
    }

    mapPage(page, i, GUEST_PHYSICAL_PAGE_FLAG, vm_mem_map);
  }

  /*for(uint64_t i = 0; i < isa_bootloader_start_addr; i+=0x1000) {
    // if(i == 0xb8000) {
    //  continue;
    // }

    uint64_t phys_page = kpalloc();
    if(phys_page == 0) {
      kpanic();
    }

    kmemset((uint8_t*)TO_HIGHER_HALF(phys_page), 0x0, 0x1000);
    mapPage(
        phys_page,
        i,
        GUEST_PHYSICAL_PAGE_FLAG,
        vm_mem_map
    );
  }*/

  //uint64_t phys_page = kpalloc();
  //mapPage(phys_page, 0xFFFF8000, 0x7, vm_mem_map); 8

  for(uint64_t i = 0; i < himem_size; i += 0x1000) {
    uint64_t phys_page = kpalloc();
    mapPage(
        phys_page,
        isa_bootloader_high_addr + i,
        GUEST_PHYSICAL_PAGE_FLAG, 
        vm_mem_map
    );
  }
  
  //TODO: create a real IOAPIC driver
  mapPage(0xfec00000, 0xfec00000, GUEST_PHYSICAL_PAGE_FLAG, vm_mem_map); // map IOAPIC
  // mapPage(0xb8000, 0xb8000, GUEST_PHYSICAL_PAGE_FLAG, vm_mem_map);
  // walkTable(0xfec00000, vm_mem_map);
  
  //uint64_t page = kpalloc();
  //if(page == 0) {
  //  kpanic();
  //}

  // mapPage(page, 0xa83e0100, 0xF, vm_mem_map);
}

// check if all the registers are restored after vmexit, if not it might be the cause for the weird thing with the pio

void init_host() {
  // allocate memory for the vmcb
  vmcb_addr = kpalloc();
  if(vmcb_addr == 0) {
    kpanic();
  }

  vmcb* vmcb_struct = (vmcb*)TO_HIGHER_HALF(vmcb_addr);
  kmemset((uint8_t*)TO_HIGHER_HALF(vmcb_addr), 0x0, sizeof(vmcb));

  host_state_area = kpalloc();
  if(host_state_area == 0) {
    kpanic();
  }

  kmemset((uint8_t*)TO_HIGHER_HALF(host_state_area), 0x0, 0x1000);
  wrmsr(0xC0010117, host_state_area);
  host_state_area = rdmsr(0xC0010117);

  // init ioio map
  ioio_map_addr = kpalloc_contignious(3);
  if(ioio_map_addr == 0) {
    kpanic();
  }

  for(auto i : list_of_ports_to_intercept) {
    uint32_t byte = i / 8;
    uint8_t bit = i % 8;
    uint8_t* ioio_map_ptr = (uint8_t*)TO_HIGHER_HALF(ioio_map_addr);
    setbit(&ioio_map_ptr[byte], bit);
  }

  uint8_t* ioio_map_ptr = (uint8_t*)TO_HIGHER_HALF(ioio_map_addr);
  // kmemset((uint8_t*)TO_HIGHER_HALF(ioio_map_addr), 0xFF, 0x1000*3);
  vmcb_struct->control.intercepts_insts_1 = (1 << 27);

  vmcb_struct->control.intercepts_insts_2 = INSTRUCTION_VMRUN_INTERCEPT;

  // init msr 
  msrpm_base_addr = kpalloc_contignious(2);
  if(msrpm_base_addr == 0) {
    kpanic();
  }

  kmemset((uint8_t*)TO_HIGHER_HALF(msrpm_base_addr), 0x0, 0x1000); 
  //vmcb_struct->control.intercept_exceptions = 0xFFFFFFFF;
  vmcb_struct->control.np_enable = 1;

  vmcb_struct->control.iopm_base_pa = ioio_map_addr;
  vmcb_struct->control.msrpm_base_pa = msrpm_base_addr;
  vmcb_struct->control.intercept_cr_reads = 0;
  vmcb_struct->control.intercepts_cr_writes = 0;
  vmcb_struct->control.intercept_exceptions = 0xFFFFFFFF;

  // create ide virtual driver
  // virtual_storage_device* storage_dev = new virtual_storage_device((char*)"ide_disk", 512);
  // ata_pio_device* ata_device = new ata_pio_device(storage_dev); 
  // create virtual cmos 
  // vmrun
}

void init_vm() {
  enable_svm();
  init_host();
  init_guest_state(1, "coreboot.rom");
  context_switching(0, 1);
  while(true) {
    // context_switching(0, 1);
    vmexit_handler();
    vmsave(host_state_area);
    svm_vmrun(&guests[curr_guest_idx].regs, vmcb_addr);
    // vmload(host_state_area);
  }
}

void vmexit_handler() {
  vmcb* vmcb_struct = (vmcb*)(TO_HIGHER_HALF(vmcb_addr));
  switch(vmcb_struct->control.exitcode) {
    case VMEXIT_IOIO : {
      handle_ioio_vmexit(); 
    }

    default: {
      break;
    }
  }
}

void handle_ioio_vmexit() {
  // determine the correct handler 
  // call the handler function
  // get the return value
  // return it to the destination register/address(if needed)
  // @TODO: add handling to REP prefix
  vmcb* vmcb_struct = (vmcb*)(TO_HIGHER_HALF(vmcb_addr));
  guest_regs* curr_guest_regs = &guests[curr_guest_idx].regs;
  uint64_t exitinfo1_val = vmcb_struct->control.exitinfo1;
  ioio_exitinfo1* exitinfo1 = std::bit_cast<ioio_exitinfo1*>(&exitinfo1_val);
  /* if(exitinfo1->port == 0x402) {
    int a = 10;
    (void)a;
  } */
  if(((exitinfo1->port >= IO_MASTER_BASE_PORT) && (exitinfo1->port <= (IO_MASTER_BASE_PORT + 7)))      ||
    ((exitinfo1->port >= IO_MASTER_BASE_CONTROL) && (exitinfo1->port <= (IO_MASTER_BASE_CONTROL + 1))) ||
    ((exitinfo1->port >= IO_SLAVE_BASE_PORT) && (exitinfo1->port <= (IO_SLAVE_BASE_PORT + 7)))         ||
    ((exitinfo1->port >= IO_SLAVE_CONTROL) && (exitinfo1->port <= (IO_SLAVE_CONTROL + 1)))  || (exitinfo1->port == 0x4242) || (exitinfo1->port == 0x4243)) {
    if(!test) {
      test = true;
      //kmemset((uint8_t*)TO_HIGHER_HALF(vmcb_struct->control.iopm_base_pa), 0xFF, 0x1000*3);
    }
      //uint32_t port = exitinfo1->port;
      //operands_decoding inst_decode = decode_in(vmcb_struct->state_save_area.rip);

      ide_transaction transaction{.exitinfo = *exitinfo1, .written_val = 0};
      if(exitinfo1->type == 0) {
        transaction.written_val = vmcb_struct->state_save_area.rax;
      }

      //uint64_t output = guests[curr_guest_idx].ide.handle_transaction(transaction);
      guests[curr_guest_idx].ata_device->dispatch_command(transaction);
  } else if(exitinfo1->port == 0x518 || exitinfo1->port == 0x514) {
    if(exitinfo1->type == 0) {

      uint32_t addr = kToLittleEndian((uint32_t)vmcb_struct->state_save_area.rax);
      uint64_t phys_addr = walkTable(addr, vmcb_struct->control.n_cr3);
      if(phys_addr == 0) {
        kpanic();
      }
      
      FwCfgDmaAccess* dma_access = std::bit_cast<FwCfgDmaAccess*>(TO_HIGHER_HALF(phys_addr));
      uint64_t dma_addr = kToLittleEndian(dma_access->address);
      uint64_t dma_phys_addr = walkTable(dma_addr, vmcb_struct->control.n_cr3);
      dma_access->address = kToLittleEndian(dma_phys_addr);

      uint32_t low_phys_addr = (uint32_t)(phys_addr & 0xFFFFFFFF);
      uint32_t high_phys_addr = (uint32_t)((phys_addr >> 32) & 0xFFFFFFFF);
      // asm volatile("outl %0,%1" : : "a"(0), "Nd"(0x514));
      if(high_phys_addr) {
        asm volatile("outl %0,%1" : : "a"(kToLittleEndian(high_phys_addr)), "Nd"(0x514));
      }

      asm volatile("outl %0,%1" : : "a"(kToLittleEndian(low_phys_addr)), "Nd"(0x518));
      dma_access->address = kToLittleEndian(dma_addr);
      //kmemset((uint8_t*)TO_HIGHER_HALF(low_phys_addr), 0x0, 8);
      //asm volatile("outl %0,%1" : : "a"(0), "Nd"(0x514));
      // get physical address and write that instead of the given nested virtual address for the dma to actually work   
    }

  } else {
    // uint64_t inst_phys_addr = walkTable(vmcb_struct->state_save_area.rip, vmcb_struct->control.n_cr3);

      if(exitinfo1->type == 0) {
        // @TODO: add support for REP
        if(exitinfo1->sz8) {
          if(exitinfo1->str) {
            // uint64_t ds = 0;
            // uint64_t rsi = 0;
            // asm volatile("mov %%ds, %0" : "a"(ds));
            //asm volatile("mov %0, %%ds" :: "a"(vmcb_struct->state_save_area.ds.selector));
            asm volatile("mov %0, %%rsi" :: "a"(curr_guest_regs->rsi));
            asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port));
            asm volatile("outsb");
          } else {
            asm volatile("outb %0,%1" :: "a"((uint8_t)(vmcb_struct->state_save_area.rax & 0xFF)), "Nd"(exitinfo1->port));
          }

        } else if(exitinfo1->sz16) {
          if(exitinfo1->str) {
            //asm volatile("mov %0, %%ds" :: "a"(vmcb_struct->state_save_area.ds.selector));
            asm volatile("mov %0, %%rsi" :: "a"(curr_guest_regs->rsi));
            asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port));
            asm volatile("outsw");
          } else {
            asm volatile("outw %0,%1" :: "a"((uint16_t)(vmcb_struct->state_save_area.rax & 0xFFFF)), "Nd"(exitinfo1->port));
          }

        } else if(exitinfo1->sz32) {
          if(exitinfo1->str) {
            //asm volatile("mov %0, %%ds" :: "a"(vmcb_struct->state_save_area.ds.selector));
            asm volatile("mov %0, %%rsi" :: "a"(curr_guest_regs->rsi));
            asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port));
            asm volatile("outsl");
          } else {
            asm volatile("outl %0,%1" :: "a"((uint32_t)(vmcb_struct->state_save_area.rax & 0xFFFFFFFF)), "Nd"(exitinfo1->port));
          }

        }

      } else {
        // handle REP read
        
        uint64_t value = 0;
        size_t iterations = exitinfo1->rep ? curr_guest_regs->rcx : 1;

        for(size_t i = 0; i < iterations; i++) {
          if(exitinfo1->sz8) {
            uint8_t read_val = 0;
            
            if(exitinfo1->str) {
              asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port) : "%rdx");
              asm volatile("mov %0, %%rdi" :: "a"((uint64_t)&read_val) : "%rdi");
              asm volatile("insb");
              uint64_t target_addr = TO_HIGHER_HALF(calculate_physical_addr(
                (uint32_t)vmcb_struct->state_save_area.es.base,
                (uint32_t)curr_guest_regs->rdi
              )); 
              //uint64_t target_addr = TO_HIGHER_HALF(vmcb_struct->state_save_area.rax);

              kmemcpy((uint8_t*)(target_addr + i), (uint8_t*)&read_val, sizeof(read_val));

            } else {
              asm volatile("inb %1,%0" : "=a"(read_val) : "Nd"(exitinfo1->port));
              value |= (read_val << (8*i));
            }

          } else if(exitinfo1->sz16) {
            uint16_t read_val = 0;

            if(exitinfo1->str) {
              asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port) : "%rdx");
              asm volatile("mov %0, %%rdi" :: "a"((uint64_t)&read_val) : "%rdi");
              asm volatile("insw");
              uint64_t target_addr = TO_HIGHER_HALF(calculate_physical_addr(
                  (uint32_t)vmcb_struct->state_save_area.es.base,
                  (uint32_t)curr_guest_regs->rdi
              ));
              //uint64_t target_addr = TO_HIGHER_HALF(vmcb_struct->state_save_area.rax);

              kmemcpy((uint8_t*)&read_val, (uint8_t*)(target_addr + i), sizeof(read_val));
            } else {
              asm volatile("inw %1,%0" : "=a"(read_val) : "Nd"(exitinfo1->port));
              value |= (read_val << (16*i));
            }

          } else if(exitinfo1->sz32) {
            uint32_t read_val = 0;

            if(exitinfo1->str) {
              asm volatile("mov %0, %%rdx" :: "a"((uint64_t)exitinfo1->port) : "%rdx");
              asm volatile("mov %0, %%rdi" :: "a"((uint64_t)&read_val) : "%rdi");
              asm volatile("insl");
              uint64_t target_addr = TO_HIGHER_HALF(walkTable(curr_guest_regs->rdi, guests[curr_guest_idx].guest_cr3));
              kmemcpy((uint8_t*)(target_addr + i), (uint8_t*)&read_val, sizeof(read_val));
            } else {
              asm volatile("inl %1,%0" : "=a"(read_val) : "Nd"(exitinfo1->port));
              value |= (read_val << (32*i));
            }
          }
        }
        
        if(!exitinfo1->str) {
          vmcb_struct->state_save_area.rax = value;
        }

        if(exitinfo1->rep == 1) {
          curr_guest_regs->rcx = 0;
        }
      }
  }

  vmcb_struct->state_save_area.rip = vmcb_struct->control.exitinfo2;
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
  vmcb* vmcb_struct = (vmcb*)TO_HIGHER_HALF(vmcb_addr);
  switch (reg) {
    case RAX:
      vmcb_struct->state_save_area.rax = value;
      guests[curr_guest_idx].guest.rax = value;

    default:
      break;
  }
}

// for some reason, after fixing the stack frame, it will use port 0x608 instead of 0xe408 for the pm_timer, it seems like it cause the problem because seabios tries to read from it in an infinite loop and it doesn't get any feedback, so it just stays inside this loop
