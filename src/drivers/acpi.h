#pragma once

#include <cstdint>

struct DESCRIPTION_HEADER {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct RSDP {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_addr;
  uint32_t length;
  uint64_t xsdt_addr;
  uint8_t extended_checksum;
  uint8_t reserved[3];
} __attribute__((packed));

struct XSDT {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint64_t entries[];
} __attribute__((packed));

struct RSDT {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint32_t entries[];
} __attribute__((packed));

struct MCFG_ENTRY {
  uint64_t base_addr;
  uint16_t pci_seg_group_num;
  uint8_t start_pci_bus_num;
  uint8_t end_pci_bus_num;
  uint32_t reserved;
} __attribute__((packed));

struct MCFG {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  char oem_revision[4];
  char creator_id[4];
  char creator_revision[4];
  uint64_t reserved;
  MCFG_ENTRY entries[];
} __attribute__((packed));

struct ACPI_SDT_HEADER {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct generic_address_structure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} __attribute__((packed));

struct FADT
{
    struct ACPI_SDT_HEADER sdt_hdr;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;

    uint8_t  preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t PM2ControlBlock;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t  pm1_event_length;
    uint8_t  pm1_control_length;
    uint8_t  pm2_control_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_length;
    uint8_t  gpe1_length;
    uint8_t  gpe1_base;
    uint8_t  cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century;

    uint16_t boot_architecture_flags;

    uint8_t  reserved2;
    uint32_t flags;

    generic_address_structure reset_reg;

    uint8_t  reset_value;
    uint8_t  reserved3[3];
  
    uint64_t                x_firmware_control;
    uint64_t                x_dsdt;

    generic_address_structure x_pm1a_event_block;
    generic_address_structure x_pm1b_event_block;
    generic_address_structure x_pm1a_control_block;
    generic_address_structure x_pm1b_control_block;
    generic_address_structure x_pm2_control_block;
    generic_address_structure x_pm_timer_block;
    generic_address_structure x_gpe0_block;
    generic_address_structure x_gpe1_block;
};

void init_acpi(uint64_t rsdp_address);

MCFG* get_mcfg();
FADT* get_fadt();
