#pragma once

#include "../vmcs/vmcs.h"
#include "../mm/npaging.h"
#include <cstdint>

#define IO_MASTER_BASE_PORT    0x1F0
#define IO_MASTER_BASE_CONTROL 0x3F6

#define IO_SLAVE_BASE_PORT     0x170
#define IO_SLAVE_CONTROL       0x376

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

  uint64_t handle_transaction(ide_transaction transaction);
};
