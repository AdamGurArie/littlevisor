#pragma once

#include "../drivers/storage_device.h"
//#include "ide.h"
#include <cstdint>

struct ata_pio_registers_s {
  uint16_t data_reg;
  uint16_t error_reg;
  uint16_t features_reg;
  uint16_t sec_count_reg;
  uint16_t sec_num_reg;
  uint16_t cylinder_high_reg;
  uint8_t drive_head_reg;
  uint8_t status_reg;
  uint8_t cmd_reg;
};

enum transfer_type_e {
  READ_SECTORS,
  WRITE_SECTORS
};

enum ata_pio_base_ports {
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

class ata_pio_device {
  private:
    ata_pio_registers_s registers;
    transfer_type_e transfer_type;
    bool device_in_transfer;
    storage_device storage_dev;
    uint8_t* transfer_buff;
    uint32_t buff_offset;
    uint32_t size_remaining;

  private:
    uint16_t handle_read_register(ata_pio_base_ports port);
    void handle_write_register(ata_pio_base_ports port, uint16_t data);
    void handle_read_sector(ide_transaction transaction);
    void handle_write_sector(ide_transaction transaction);
    void handle_read_data(ide_transaction transaction);
    void handle_write_data(ide_transaction transaction);
    void handle_command(uint16_t command);
 

  public:
    ata_pio_device() = default;
    uint64_t dispatch_command(ide_transaction transaction);
};