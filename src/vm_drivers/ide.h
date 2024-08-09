#pragma once

#include "../mm/npaging.h"
#include <cstdint>

#define IO_MASTER_BASE_PORT    0x1F0
#define IO_MASTER_BASE_CONTROL 0x3F6

#define IO_SLAVE_BASE_PORT     0x170
#define IO_SLAVE_CONTROL       0x376

#define SECTOR_SIZE 512

enum ide_base_ports {
  DATA_REGISTER,
  ERROR_REGISTER,
  FEATURES_REGISTER,
  SECTOR_COUNT_REGISTER,
  SECTOR_NUMBER_REGISTER_LBALOW,
  CYLINDER_LOW_REGISTER_LBAMID,
  CYLINDER_HIGH_REGISTER_LBAHIGH,
  DRIVE_HEAD_REGISTER,
  STATUS_REGISTER,
  COMMAND_REGISTER,
};

enum ide_control_ports {
  ALTERNATE_STATUS_REGISTER,
  DEVICE_CONTROL_REGISTER,
  DRIVE_ADDRESS_REGISTER
};

struct ioio_exitinfo1 {
  uint8_t type : 1;
  uint8_t reserved1 : 1;
  uint8_t str : 1;
  uint8_t rep : 1;
  uint8_t sz8 : 1;
  uint8_t sz16 : 1;
  uint8_t sz32 : 1;
  uint8_t a16 : 1;
  uint8_t a32 : 1;
  uint8_t a64 : 1;
  uint8_t seg : 3;
  uint8_t reserved2 : 3;
  uint16_t port;
} __attribute__((packed));

struct ide_transaction {
  ioio_exitinfo1 exitinfo;
  uint64_t written_val;
};

class ideDevice {
  private:
  bool virtualized;
  // IO base registers
  uint16_t data_register;
  uint16_t error_register;
  uint16_t features_register;
  uint16_t sector_count_register;
  uint16_t sector_number_register_lbalow;
  uint16_t cylinder_low_register_lbamid;
  uint16_t cylinder_high_register_lbahigh;
  uint8_t drive_head_register;
  uint8_t status_register;
  uint8_t command_register;
  uint8_t registers[12];

  //Control base registers
  uint8_t alter_status_register;
  uint8_t device_control_register;
  uint8_t drive_address_register;

  private:
  void handle_registerWrite(ide_transaction transaction);

  uint64_t handle_registerRead(ide_transaction transaction);

  void handle_command();

  uint64_t passthrough_transaction(ide_transaction transaction);

  void handle_readSector();

  void handle_writeSector();

  public:
  ideDevice(bool virtualized) : virtualized(virtualized) {
    mapPage(IO_MASTER_BASE_PORT, IO_MASTER_BASE_PORT, 0x0);
    mapPage(IO_MASTER_BASE_CONTROL, IO_MASTER_BASE_CONTROL, 0x0);
    mapPage(IO_SLAVE_BASE_PORT, IO_SLAVE_BASE_PORT, 0x0);
    mapPage(IO_SLAVE_CONTROL, IO_SLAVE_CONTROL, 0x0);
  }

  ideDevice() {
    mapPage(IO_MASTER_BASE_PORT, IO_MASTER_BASE_PORT, 0x0);
    mapPage(IO_MASTER_BASE_CONTROL, IO_MASTER_BASE_CONTROL, 0x0);
    mapPage(IO_SLAVE_BASE_PORT, IO_SLAVE_BASE_PORT, 0x0);
    mapPage(IO_SLAVE_CONTROL, IO_SLAVE_CONTROL, 0x0);
  }

  uint64_t handle_transaction(ide_transaction transaction);

  inline void set_virtualized_flag(bool virtualized) {
    this->virtualized = virtualized;
  }
};
