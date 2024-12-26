#include "drivers/ram_disk.h"
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
  init_pmm(memmap_req.response);
  init_idt();
  // while(true) {
  //   asm volatile("out %0, $0xE9" :: "a"('a'));
  // }

  init_acpi((uint64_t)rsdp_request.response->address);
  MCFG* mcfg = get_mcfg();
  init_pci((uint64_t)mcfg);
  init_heap();
  asm volatile("mov $0xffff8000000b8000, %rax");
  asm volatile("mov $0x0F61, (%rax)");
  // int i = 1/0;
  // (void)i;
  // init_ahci();
  //asm volatile("int3");
  limine_file* limine_f = module_request.response->modules[0];
  save_host_pageMap();
  ramDisk* ramdisk = new ramDisk((uintptr_t)limine_f->address);
  init_fs(ramdisk);
  // char file_buff[16];
  // const char* file_name = "bios12.bin";
  // uint32_t fd = vopenFile(file_name);
  // uint64_t file_size = vgetFileSize(fd);
  //vseekp(fd, file_size - 16);
  //vreadFile(fd, file_buff, 16);
  //kpalloc();
  init_vm();

  while(1);
  //uint8_t buff[512];
  //ramdisk.read_sector(buff, 0, 1);
}
