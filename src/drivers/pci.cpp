#include "pci.h"
#include "acpi.h"
#include "../common.h"
#include <cstdint>
#include <bit>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_DATA_ADDR 0xCFC

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

bool find_device(pci_device_descriptor* descriptor, uint8_t classcode, uint8_t subclass, uint8_t prog_if) {
  for(uint32_t bus = 0; bus < 256; bus++) {
    for(uint32_t device = 0; device < 256; device++) {
      uint16_t vendor_id = read_configuration_dword(bus, device, 0, VENDOR_ID);
      if(vendor_id == 0xFFFF) {
        continue; // Device doesn't exits
      }

      uint16_t header_type = read_configuration_dword(bus, device, 0, HEADER_TYPE);
      uint16_t curr_classcode = read_device_identifier(bus, device, 0, CLASS_CODE);
      uint16_t curr_subclass = read_device_identifier(bus, device, 0, SUBCLASS);
      uint16_t curr_prog_if = read_device_identifier(bus, device, 0, PROG_IF);

      if(
        curr_classcode == classcode &&
        curr_subclass == subclass &&
        curr_prog_if == prog_if
      ) {

        descriptor->bus = bus;
        descriptor->device = device;
        descriptor->function = 0;
        return true;

      }

      if(header_type & 0x80) {

        for(uint8_t function = 0; function < 8; function++) {

          vendor_id = read_configuration_dword(bus, device, function, VENDOR_ID);
          if(vendor_id == 0xFFFF) {

            continue;
          }

          if(
              read_configuration_dword(bus, device, function, CLASS_CODE) == classcode &&
              read_configuration_dword(bus, device, function, SUBCLASS) == subclass &&
              read_configuration_dword(bus, device, function, PROG_IF) == prog_if
            ) {

            descriptor->bus = bus;
            descriptor->device = device;
            descriptor->function = function;
            return true;
          }

        }

      }

    }

  }

  return false;

}

uint32_t get_header_bar(pci_device_descriptor* descriptor, uint8_t bar_number) {
  uint32_t bar = read_configuration_dword(
      descriptor->bus,
      descriptor->device,
      descriptor->function,
      BAR0 + bar_number * 4
  ); // TODO: need to read dword not word!!!!
  
  if((bar & 0x1) == 0) {
  
    if(((bar >> 1) & 0x2) == 0x0) {

      return bar & 0xFFFFFFF0; // TODO: handle 64bit_cast

    } else if(((bar >> 1) & 0x2) == 0x2) {

      return bar & 0xFFFFFFF0;

    }

  } else {

    return bar & 0xFFFFFFFC;

  }
  
  return 0;
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

uint32_t read_configuration_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  uint32_t pci_config_req = offset | (func << 16) | (slot << 11) | (bus << 16) |  (0x80 << 24);

  write_to_port(PCI_CONFIG_ADDR, pci_config_req);
  
  return (read_from_port_dword(PCI_DATA_ADDR) >> ((offset & 2) * 8) & 0xFFFF);
}

void write_configuration_word(pci_device_descriptor* device_descriptor, uint8_t offset, uint16_t data) {
  uint32_t pci_config_req = offset | (device_descriptor->function << 16) | (device_descriptor->device << 11) | (device_descriptor->bus << 16) | (0x80 << 24);

  write_to_port(PCI_CONFIG_ADDR, pci_config_req);
  write_to_port(PCI_DATA_ADDR, data);
}

uint32_t read_configuration_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  uint32_t pci_config_req = offset | (func << 16) | (slot << 11) | (bus << 16) | (0x80 << 24);

  write_to_port(PCI_CONFIG_ADDR, pci_config_req);

  return read_from_port_dword(PCI_DATA_ADDR);
}

uint8_t read_device_identifier(uint8_t bus, uint8_t slot, uint8_t func, HEADER_TYPE_0_OFFSETS offset) {
  uint32_t pci_config_req = offset | (func << 16) | (slot << 11) | (bus << 16) | (0x80 << 24);

  write_to_port(PCI_CONFIG_ADDR, pci_config_req);
  
  uint8_t data = read_from_port_byte(PCI_DATA_ADDR + (offset % 4));
  return data;
}

void pci_enable_interrupts(pci_device_descriptor* device_descriptor) {
  uint16_t command_register = read_configuration_word(
      device_descriptor->bus,
      device_descriptor->device,
      device_descriptor->function,
      COMMAND
  );

  clearbit(&command_register, 10);
  write_configuration_word(device_descriptor, COMMAND, command_register);
}

void pci_enable_dma(pci_device_descriptor* device_descriptor) {
  uint16_t command_register = read_configuration_word(
      device_descriptor->bus,
      device_descriptor->device,
      device_descriptor->function,
      COMMAND
  );

  setbit(&command_register, 10);
  write_configuration_word(device_descriptor, COMMAND, command_register);
}

void pci_enable_bus_mastering(pci_device_descriptor* device_descriptor) {
  uint16_t command_register = read_configuration_word(
      device_descriptor->bus,
      device_descriptor->device,
      device_descriptor->function,
      COMMAND
  );

  setbit(&command_register, 2);
  write_configuration_word(device_descriptor, COMMAND, command_register);
}

