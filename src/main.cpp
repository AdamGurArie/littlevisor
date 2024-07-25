#include  "limine.h"
#include "common.h"
#include "vmx.h"
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
  uint64_t vmxon_region = kpalloc();
  //enable_vmx(vmxon_region);
  init_idt();
  int a = 1/0;
  write_to_port(0xe9, a);
  while(true);
  asm volatile("hlt");
  //uint8_t a[] = "hello";
  //while(true) {
  
  //  write_to_port(0xe9, (uint8_t)'r');
  //}
  //asm volatile("hlt");
  //while(true);
}
