#pragma once

#include <cstdint>
#include <stdint.h>
//static MCFG* mcfg = (MCFG*)0;

struct status_register {
  uint8_t reserved_1 : 2;
  uint8_t interrupt_status : 1;
  uint8_t capabilities_list : 1;
  uint8_t capable_66_mhz : 1;
  uint8_t reserved_2 : 1;
  uint8_t fast_btb_capable : 1;
  uint8_t master_data_parity_error : 1;
  uint8_t devsel_timing : 2;
  uint8_t signaled_target_abort : 1;
  uint8_t received_target_abort : 1;
  uint8_t received_master_abort : 1;
  uint8_t signaled_system_error : 1;
  uint8_t detected_parity_error : 1;
} __attribute__((packed));

struct command_register {
  uint8_t io_space : 1;
  uint8_t memory_space : 1;
  uint8_t bus_master : 1;
  uint8_t special_cycles : 1;
  uint8_t mem_write_and_invlidate_enable : 1;
  uint8_t vga_pallete_snoop : 1;
  uint8_t parity_error_response : 1;
  uint8_t reserved_1 : 1;
  uint8_t serr_enable : 1;
  uint8_t fast_btb_enable : 1;
  uint8_t interrupt_disable : 1;
  uint8_t reserved_2 : 1;
} __attribute__((packed));

struct common_pci_header {
  uint16_t vendor_id;
  uint16_t device_id;
  command_register cmd_reg;
  status_register status_reg;
  uint8_t revision_id;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class_code;
  uint8_t cache_line_size;
  uint8_t latency_timer;
  uint8_t header_type;
  uint8_t bist;
} __attribute__((packed));

struct header_type_0 {
  common_pci_header header;
  uint32_t bar0;
  uint32_t bar1;
  uint32_t bar2;
  uint32_t bar3;
  uint32_t bar4;
  uint32_t bar5;
  uint32_t cardbus_cis_ptr;
  uint16_t subsystem_vendor_id;
  uint32_t subsystem_id;
  uint32_t expansion_rom_base_addr;
  uint8_t capabilities_ptr;
  uint8_t reserved_1[3];
  uint32_t reserved_2;
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint8_t min_grant;
  uint8_t max_latency;
} __attribute__((packed));

struct header_type_1 {
  common_pci_header header;
  uint32_t bar0;
  uint32_t bar1;
  uint8_t primary_bus_number;
  uint8_t secondary_bus_number;
  uint8_t subordinate_bus_number;
  uint8_t secondary_latency_timer;
  uint8_t io_base;
  uint8_t io_limit;
  uint16_t secondary_status;
  uint16_t memory_base;
  uint16_t memory_limit;
  uint16_t prefetchable_mem_base;
  uint16_t prefetchable_mem_limit;
  uint32_t prefetchable_base_upper;
  uint32_t prefetchable_limit_upper;
  uint16_t io_base_upper;
  uint16_t io_limit_upper;
  uint8_t capability_ptr;
  uint8_t reserved_1[3];
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint16_t bridge_control;
} __attribute__((packed));

struct header_type_2 {
  common_pci_header header;
  uint32_t cardbus_socket_exca_base_addr;
  uint8_t offset_of_capabilities_list;
  uint8_t reserved;
  uint16_t secondary_status;
  uint8_t pci_bus_number;
  uint8_t cardbus_number;
  uint8_t subordinate_bus_number;
  uint8_t cardbus_latency_timer;
  uint32_t memory_base_address_0;
  uint32_t memory_limit_0;
  uint32_t memory_base_address_1;
  uint32_t memory_limit_1;
  uint32_t io_base_addr_0;
  uint32_t io_limit_0;
  uint32_t io_base_addr_1;
  uint32_t io_limit_1;
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint16_t bridge_control;
  uint16_t subsystem_device_id;
  uint16_t subsystem_vendor_id;
  uint32_t pc_card_legacy_mode_base_addr;
} __attribute__((packed));

void init_pci(uint64_t mcfg_ptr);
common_pci_header* find_device(uint8_t classcode, uint8_t subclass, uint8_t prog_if);
common_pci_header* access_device_header(uint8_t segment_group, uint8_t bus, uint8_t device, uint8_t function);
