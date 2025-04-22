#include "drivers/ram_disk.h"
#include "drivers/storage_device.h"
#include "fs/fat32.h"
#include "fs/vfs.h"
#include "kheap.h"
#include  "limine.h"
#include "common.h"
#include "mm/npaging.h"
#include "vmcs/vmcs.h"
#include "drivers/acpi.h"
#include "drivers/pci.h"
//#include "drivers/ahci.h"
#include "drivers/ram_disk.h"
#include <cstdint>
#include <cstring>
#include "pmm.h"
#include "interrupts.h"
#include "drivers/ahci.h"
#include "drivers/nvme.h"
#include "mm/paging.h"
#include "gdt.h"
#include "timers/pit.h"
#include "scheduler/scheduler.h"
#include "elf/elfloader.h"

// @TODO: the name encoding algorithm of fat32 doesn't work with test.elf for example, fix that
// @TODO: check why placing elf_hdr elf_header{0} gives a crash

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_req = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0,
  .response = 0,
};

__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
  .id = LIMINE_RSDP_REQUEST,
  .revision = 0,
  .response = 0,
};

__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0,
  .response = 0,
  .internal_module_count = 0,
  .internal_modules = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_address_request kernel_addr_request {
  .id = LIMINE_KERNEL_ADDRESS_REQUEST,
  .revision = 0,
  .response = 0,
};

__attribute__((used, section(".requests")))
static volatile struct limine_kernel_file_request kernel_file_request {
  .id = LIMINE_KERNEL_FILE_REQUEST,
  .revision = 0,
  .response = 0,
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

void memcpy(uint8_t* dest, uint8_t* src, uint32_t size) {
  for(uint32_t i = 0; i < size; i++) {
    dest[i] = src[i];
  }
}

void _start() {
  init_gdt();
  init_idt();
  init_pit();
  init_pmm(memmap_req.response);
  //init_idt();
  // while(true) {
  //   asm volatile("out %0, $0xE9" :: "a"('a'));
  // }

  init_acpi((uint64_t)rsdp_request.response->address);
  MCFG* mcfg = get_mcfg();
  init_pci((uint64_t)mcfg);
  init_heap();
  init_vmm(memmap_req.response, kernel_file_request.response->kernel_file, kernel_addr_request.response);
  // init_vmm(memmap_req.response, kernel_file_request.response->kernel_file, kernel_addr_request.response);
  uint64_t virt_addr = get_host_pageMap();
  uint64_t phys_addr = kpalloc();
  //allocate_page(&virt_addr, &phys_addr, PRESENT_FLAG | RW_FLAG);
  kmemset((uint8_t*)TO_HIGHER_HALF(phys_addr), 0x0, 0x200);
  phys_addr = walk_host_tables(TO_HIGHER_HALF(phys_addr));
  // mapPage(0x2c0001000, 0xffff8002c0001000, RW_FLAG | PRESENT_FLAG, 0);
  // mapPage(0x2c0000000, 0xffff8002c0000000, RW_FLAG | PRESENT_FLAG, 0);
  // mapPage(0x2c0002000, 0xffff8002c0002000, RW_FLAG | PRESENT_FLAG, 0);
  // map_host_page((uint64_t)&mcfg, phys_addr, PRESENT_FLAG | RW_FLAG);

  // asm volatile("mov $0xffff8000000b8000, %rax");
  // asm volatile("mov $0x0F61, (%rax)");
  // int i = 1/0;
  // (void)i;
  uint8_t buff[512] = {0};
  kmemset(buff, 0x45, 512);
  ahci ahci_dev = ahci();
  //ahci_dev.write_sector(0, buff);
  //ahci_dev.write_sector(1, buff);
  //ahci_dev.write_sector(2, buff);
  //ahci_dev.commit_transaction(buff, 1, 1, false);
  //kmemset(buff, 0xff, 512);
  //ahci_dev.write_data(buff, 256, 512);
  //kmemset(buff, 0x0, 512);
  //ahci_dev.read_data(buff, 256, 512);
  //init_nvme();
  
  //asm volatile("int3");
  // init_scheduler(); 
  limine_file* limine_f = module_request.response->modules[0];
  map_range(0, (uint64_t)limine_f->address, TO_LOWER_HALF((uint64_t)limine_f->address), (limine_f->size + PAGE_SIZE) / PAGE_SIZE, RW_FLAG | PRESENT_FLAG);
  //save_host_pageMap();
  ramDisk* ramdisk = new ramDisk((uintptr_t)limine_f->address, limine_f->size);
  init_fs(&ahci_dev);

  uint32_t test_fd = vopenFile("test.elf");
  create_user_task(test_fd);
  init_scheduler();
  //uint64_t entry = load_elf_to_memory(test_fd, 0);
  //void (*elf)() = (void(*)())entry;
  //elf();
  //kpalloc();
  //init_vm();

  while(1);
  //uint8_t buff[512];
  //ramdisk.read_sector(buff, 0, 1);
}
