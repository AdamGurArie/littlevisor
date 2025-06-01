#include "acpi.h"
#include "../common.h"
#include "../mm/npaging.h"
#include <bit>
#include <cstdint>
#include <cstring>

#define MCFG_SIG "MCFG"
#define FADT_SIG "FACP"

static uint64_t rsdp_addr = 0;

void init_acpi(uint64_t rsdp_address) {
  rsdp_addr = rsdp_address;
  mapPage(TO_LOWER_HALF(rsdp_addr), rsdp_addr, 0x03, 0);
  RSDP* rsdp = std::bit_cast<RSDP*>(rsdp_addr);
  if(rsdp->xsdt_addr == 0x0) {
    mapPage(rsdp->rsdt_addr, TO_HIGHER_HALF(rsdp->rsdt_addr), 0x03, 0);
  } else {
    mapPage(rsdp->xsdt_addr, TO_HIGHER_HALF(rsdp->xsdt_addr), 0x03, 0);
  }
}

MCFG* get_mcfg() {
  RSDP* rsdp = std::bit_cast<RSDP*>(rsdp_addr);
  if(rsdp->xsdt_addr == 0x0) {
    //use rsdt
    RSDT* rsdt = std::bit_cast<RSDT*>(TO_HIGHER_HALF(rsdp->rsdt_addr));
    uint32_t num_of_entries = (rsdt->length - 36) / sizeof(uint32_t);
    for(uint32_t i = 0; i < num_of_entries; i++) {
      DESCRIPTION_HEADER* header = std::bit_cast<DESCRIPTION_HEADER*>(TO_HIGHER_HALF(rsdt->entries[i]));
      if(kmemcmp((uint8_t*)header->signature, (uint8_t*)MCFG_SIG, strlen(MCFG_SIG)) == 0) {
        return std::bit_cast<MCFG*>(header);
      }
    }

  } else {
    //use xsdt
    XSDT* xsdt = std::bit_cast<XSDT*>(TO_HIGHER_HALF(rsdp->xsdt_addr));
    uint32_t num_of_entries = (xsdt->length - 36) / sizeof(uint64_t);
    for(uint32_t i = 0; i < num_of_entries; i++) {
      DESCRIPTION_HEADER* header = std::bit_cast<DESCRIPTION_HEADER*>(TO_HIGHER_HALF(xsdt->entries[i]));
      if(kmemcmp((uint8_t*)header->signature, (uint8_t*)MCFG_SIG, strlen(MCFG_SIG)) == 0) {
        return std::bit_cast<MCFG*>(header);
      } 
    }
  }

  return (MCFG*)0;
}

FADT* get_fadt() {
  RSDP* rsdp = std::bit_cast<RSDP*>(rsdp_addr);
  if(rsdp->xsdt_addr == 0x0) {
    RSDT* rsdt = std::bit_cast<RSDT*>(TO_HIGHER_HALF(rsdp->rsdt_addr));
    uint32_t num_of_entries = (rsdt->length - 36) / sizeof(uint32_t);
    for(uint32_t i = 0; i < num_of_entries; i++) {
      DESCRIPTION_HEADER* header = std::bit_cast<DESCRIPTION_HEADER*>(TO_HIGHER_HALF(rsdt->entries[i]));
      if(kmemcmp((uint8_t*)header->signature, (uint8_t*)FADT_SIG, strlen(FADT_SIG)) == 0) {
        return std::bit_cast<FADT*>(header);
      }
    }
  } else {
    XSDT* xsdt = std::bit_cast<XSDT*>(TO_HIGHER_HALF(rsdp->xsdt_addr));
    uint32_t num_of_entries = (xsdt->length - 36) / sizeof(uint64_t);
    for(uint32_t i = 0; i < num_of_entries; i++) {
      DESCRIPTION_HEADER* header = std::bit_cast<DESCRIPTION_HEADER*>(TO_HIGHER_HALF(xsdt->entries[i]));
      if(kmemcmp((uint8_t*)header->signature, (uint8_t*)FADT_SIG, strlen(FADT_SIG)) == 0) {
        return std::bit_cast<FADT*>(header);
      }
    }
  }

  return (FADT*)0x0;
}
