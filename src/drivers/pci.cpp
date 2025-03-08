#include "pci.h"
#include "acpi.h"
#include "../common.h"
#include <cstdint>
#include <bit>

static MCFG* mcfg = 0;
//static uint16_t max_seg_group = 0;

void init_pci(uint64_t mcfg_ptr) {
  mcfg = (MCFG*)mcfg_ptr;
}

common_pci_header* find_device(uint8_t classcode, uint8_t subclass, uint8_t prog_if) {
  MCFG_ENTRY* entries = mcfg->entries;
  uint32_t num_of_entries = (mcfg->length - 44) / sizeof(MCFG_ENTRY);
  for(uint32_t mcfg_entry = 0 ; mcfg_entry < num_of_entries; mcfg_entry++) {

    uint8_t segment_group = entries[mcfg_entry].pci_seg_group_num;
    
    for(uint32_t bus = 0; bus < 255; bus++) {
    
      for(uint32_t device = 0; device < 32; device++) {
      
        for(uint32_t function = 0; function < 8; function++) {
        
          common_pci_header* pci_header = access_device_header(segment_group, bus, device, function);
          if(pci_header == 0x0) {
            continue;
          }

          if(pci_header->class_code == classcode && pci_header->subclass == subclass && pci_header->prog_if == prog_if) {
            return pci_header;
          }
        
        }
      
      }
    
    }
  
  }

  return (common_pci_header*)0x0;
}

common_pci_header* access_device_header(uint8_t segment_group, uint8_t bus, uint8_t device, uint8_t function) {
  MCFG_ENTRY* entries = mcfg->entries;
  uint32_t num_of_entries = (mcfg->length - 44) / sizeof(MCFG_ENTRY);
  for(uint32_t i = 0; i < num_of_entries; i++) {
    if(entries[i].pci_seg_group_num == segment_group) {
      uint64_t physical_addr = entries[i].base_addr + (bus << 20 | device << 15 | function << 12);
      return std::bit_cast<common_pci_header*>(TO_HIGHER_HALF(physical_addr));
    }
  }
  
  return (common_pci_header*)0x0;
}

void enable_device_memio(uint8_t segment_group, uint8_t bus, uint8_t device, uint8_t function) {
  common_pci_header* pci_header = access_device_header(segment_group, bus, device, function);
  setbit((uint64_t*)&pci_header->cmd_reg, 1);
} 
