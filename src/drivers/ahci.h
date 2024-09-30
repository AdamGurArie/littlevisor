#pragma once

#include "pci.h"
#include "storage_device.h"
#include <cstdint>

enum port_type {
  PORT_NOT_AVAILABLE = 0,
  SATA_SIG_ATA = 0x101,
  SATA_SIG_ATAPI = 0xEB140101,
  SATA_SIG_SEMB = 0xC33C0101,
  SATA_SIG_PM = 0x96690101
};

enum fis_types {
  REG_H2D = 0x27,
  REG_D2H = 0x34,
  DMA_D2H = 0x39,
  DMA_BIDIRECTIONAL = 0x41,
  DATA = 0x46,
  BIST = 0x58,
  PIO_SETUP = 0x5F,
  DEV_BITS = 0xA1
};

struct generic_host_control_struct {
  uint32_t cap;
  uint32_t ghc;
  uint32_t is;
  uint32_t pi;
  uint32_t vs;
  uint32_t ccc_ctl;
  uint32_t ccc_ports;
  uint32_t em_loc;
  uint32_t em_ctl;
  uint32_t cap2;
  uint32_t bohc;
} __attribute__((packed));

struct port_register_struct {
  uint32_t pxclb;
  uint32_t pxclbu;
  uint32_t pxfb;
  uint32_t pxfbu;
  uint32_t pxis;
  uint32_t pxie;
  uint32_t pxcmd;
  uint32_t reserved_1;
  uint32_t pxtfd;
  uint32_t pxsig;
  uint32_t pxssts;
  uint32_t pxsctl;
  uint32_t pxserr;
  uint32_t pxsact;
  uint32_t pxci;
  uint32_t pxsntf;
  uint32_t pxfbs;
  uint32_t reserved_2[11];
  uint32_t vendor[4];
} __attribute__((packed));

struct hba_mem_regs {
  generic_host_control_struct generic_host_control; //44 bytes
  uint8_t reserved_1[116];
  uint8_t vendor_specific_registers[96];
  port_register_struct port_registers[32];
} __attribute__((packed));

struct command_header {
  uint8_t cfl : 5;
  uint8_t atapi : 1;
  uint8_t write : 1;
  uint8_t prefetchable : 1;
  uint8_t reset : 1;
  uint8_t bist : 1;
  uint8_t clear : 1;
  uint8_t reserved_1 : 1;
  uint8_t pmp : 4;
  uint16_t prdtl;
  uint32_t prdbc;
  uint32_t ctba;
  uint32_t ctbau;
  uint32_t reserved_2[4];
} __attribute__((packed));

struct prdt {
  uint8_t reserved_1 : 1;
  uint32_t dba : 31;
  uint32_t dbau;
  uint32_t reserved_2;
  uint32_t dbc : 22;
  uint16_t reserved_3 : 9;
  uint8_t ioc : 1;
} __attribute__((packed));

struct command_table {
  uint8_t cfis[64];
  uint8_t acmd[16];
  uint8_t reserved[48];
  prdt prdt_list[];
} __attribute__((packed));

struct FIS_REG_H2D {
  uint8_t fis_type;
  uint8_t pmport : 4;
  uint8_t rsv0 : 3;
  uint8_t c : 1;
  uint8_t command;
  uint8_t featurel;
  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;
  uint8_t device;
  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;
  uint8_t featureh;
  uint8_t countl;
  uint8_t counth;
  uint8_t icc;
  uint8_t control;
  uint8_t reserved[4];
} __attribute__((packed));

struct FIS_REG_D2H {
  uint8_t fis_type;
  uint8_t pmport : 4;
  uint8_t rsv0 : 2;
  uint8_t i : 1;
  uint8_t rsv1 : 1;
  uint8_t status;
  uint8_t error;
  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;
  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;
  uint8_t rsv2;
  uint8_t countl;
  uint8_t counth;
  uint8_t rsv3[2];
  uint8_t rsv4[4];
} __attribute__((packed));

class ahci : storage_device{
private:
  hba_mem_regs* ahci_hba = 0;
  port_type port_types[32] = {};
  uint32_t sata_device_port = 0;

public:


private:
  void init_port(uint32_t port);
  void map_ports();
  void init_ahci();
  void stop_command_engine(uint32_t port);
  void start_command_engine(uint32_t port);

public: 
  void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
  void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
  uint8_t read_data(uint8_t* buff, uint64_t offset, uint32_t size);
  uint8_t write_data(uint8_t* buff, uint64_t offset, uint32_t size);
  void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
  uint64_t get_sector_size();
  ahci();
};

// void init_ahci();
// void read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size);
// void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);
