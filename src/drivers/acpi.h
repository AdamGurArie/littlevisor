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
};

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
};

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
};

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
};

struct MCFG_ENTRY {
  uint64_t base_addr;
  uint16_t pci_seg_group_num;
  uint8_t start_pci_bus_num;
  uint8_t end_pci_bus_num;
  uint32_t reserved;
};

struct MCFG {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  char creator_id[4];
  char creator_revision[4];
  uint64_t reserved;
  MCFG_ENTRY entries[];
};

void init_acpi(uint64_t rsdp);

MCFG* get_mcfg();
