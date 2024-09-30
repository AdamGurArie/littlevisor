#include "drivers/ram_disk.h"
#include  "limine.h"
#include "common.h"
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

  init_acpi((uint64_t)rsdp_request.response->address);
  MCFG* mcfg = get_mcfg();
  init_pci((uint64_t)mcfg);
  // init_ahci();
  limine_file* limine_f = module_request.response->modules[0];
  ramDisk ramdisk = ramDisk((uintptr_t)limine_f->address);
  uint8_t buff[512];
  ramdisk.read_sector(buff, 0, 1);
}
