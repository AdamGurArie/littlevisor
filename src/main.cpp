#include  "limine.h"
#include "common.h"
#include "vmcs/vmcs.h"
#include "drivers/acpi.h"
#include "drivers/pci.h"
#include "drivers/ahci.h"
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
  init_ahci();
  uint8_t read_buff[512] = {};
  //kmemset(read_buff, 0x1, 512);
  //write_to_disk(read_buff, 0, 512);
  //kmemset(read_buff, 0x0, 512);
  //read_from_disk(read_buff, 0, 512);
  uint8_t* write_buff = (uint8_t*)0x2000;
  kmemset(TO_HIGHER_HALF(write_buff), 0x1, 512);
  commit_transaction((uint8_t*)0x2000, 0, 1, true);
  kmemset(TO_HIGHER_HALF(write_buff), 0x0, 512);
  commit_transaction((uint8_t*)0x2000, 0, 1, false);
  //uint64_t vmxon_region = kpalloc();
  //enable_vmx(vmxon_region);
  init_vm();
  uint32_t size = sizeof(vmcb_control);
  (void)size;
  //write_to_port(0xe9, a);
  while(true);
  //uint8_t a[] = "hello";
  //while(true) {
  
  //  write_to_port(0xe9, (uint8_t)'r');
  //}
  //asm volatile("hlt");
  //while(true);
}
