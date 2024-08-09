#include  "limine.h"
#include "common.h"
#include "vmcs/vmcs.h"
#include "drivers/acpi.h"
#include "drivers/pci.h"
#include <cstdint>
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
  init_acpi((uint64_t)rsdp_request.response->address);
  MCFG* mcfg = get_mcfg();
  init_pci((uint64_t)mcfg);
  //uint64_t vmxon_region = kpalloc();
  //enable_vmx(vmxon_region);
  init_idt();
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
