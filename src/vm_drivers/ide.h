#pragma once
/**
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

enum status_register {
  ST_REG_ERR,
  ST_REG_IDX,
  ST_REG_CORR,
  ST_REG_DRQ,
  ST_REG_SRV,
  ST_REG_DF,
  ST_REG_RDY,
  ST_REG_BSY
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
  uint32_t fd;
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
  uint64_t size_of_data_to_transfer;
  uint64_t offset_in_data;
  uint8_t* buffer_with_data;
  uint8_t ready_to_trasnfer;

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
  uint16_t read_data_io();
  void write_data_io(uint16_t data);

  public:
  ideDevice(bool virtualized, uint32_t fd) : virtualized(virtualized), fd(fd) {
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
  uint8_t check_if_ready_for_transfer();

  inline void set_virtualized_flag(bool virtualized) {
    this->virtualized = virtualized;
  }
};**/
