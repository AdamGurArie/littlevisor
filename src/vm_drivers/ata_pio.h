#pragma once

#include "../drivers/storage_device.h"
//#include "ide.h"
#include <cstdint>

#define IO_MASTER_BASE_PORT    0x1F0
#define IO_MASTER_BASE_CONTROL 0x3F6

#define IO_SLAVE_BASE_PORT     0x170
#define IO_SLAVE_CONTROL       0x376

struct ata_response {
  uint8_t* buffer;
  uint32_t size;
};

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
  uint8_t alt_status_reg;
  uint8_t dev_ctrl_reg;
  uint8_t drive_addr_reg;
};

enum transfer_type_e {
  READ_SECTORS,
  WRITE_SECTORS,
  IDENTIFY_CMD
};

enum ata_pio_base_ports {
  DATA_REGISTER,
  ERROR_FEATURES_REGISTER,
  SECTOR_COUNT_REGISTER,
  SECTOR_NUMBER_REGISTER_LBALOW,
  CYLINDER_LOW_REGISTER_LBAMID,
  CYLINDER_HIGH_REGISTER_LBAHIGH,
  DRIVE_HEAD_REGISTER,
  STATUS_COMMAND_REGISTER,
  ALT_STATUS_REG_DEV_CTRL = 0x3F6,
  DRIVE_ADDR_REG
};

enum DEV_CTRL_DEV_BITS {
  DEV_CTRL_NA1,
  DEV_CTRL_NIEN,
  DEV_CTRL_SRST,
  DEV_CTRL_NA2,
  DEV_CTRL_NA3,
  DEV_CTRL_NA4,
  DEV_CTRL_NA5,
  DEV_CTRL_HOB
};

enum STATUS_REG_BITS {
  STATUS_REG_ERR,
  STATUS_REG_IDX,
  STATUS_REG_CORR,
  STATUS_REG_DRQ,
  STATUS_REG_SRV,
  STATUS_REG_DF,
  STATUS_REG_RDY,
  STATUS_REG_BSY
};

/*enum ata_pio_read_ports {
  DATA_REGISTER,
  ERROR_REGISTER,
  SECTOR_COUNT_REGISTER,
  SECTOR_NUMBER_REGISTER_LBALOW,
  CYLINDER_LOW_REGISTER_LBAMID,
  CYLINDER_HIGH_REGISTER_LBAHIGH,
  DRIVE_HEAD_REGISTER,
  STATUS_REGISTER
};*/

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
    storage_device* storage_dev;
    uint8_t* transfer_buff;
    uint32_t buff_offset;
    uint32_t size_remaining;

  private:
    uint16_t handle_read_register(ata_pio_base_ports port);
    void handle_write_register(ata_pio_base_ports port, uint16_t data);
    void handle_read_sectors();
    void handle_write_sectors();
    void handle_read_data(ide_transaction transaction, ata_response* response);
    void handle_write_data(ide_transaction transaction);
    void handle_command(uint16_t command);
    void handle_identify_command();
    void handle_device_ctrl_reg_write();
    void handle_device_reset();
 

  public:
    ata_pio_device(storage_device* virt_storage_device) : storage_dev(virt_storage_device) {};
    void dispatch_command(ide_transaction transaction, ata_response* response);
};

class virtual_storage_device : public storage_device {
  private:
    uint32_t sector_size;
    const char* file_name;

  public:

  virtual_storage_device(const char* file_name, uint32_t sector_size) : sector_size(sector_size), file_name(file_name) {};
  uint8_t read_sector(uint32_t sector_number, uint8_t* buff);
  uint8_t write_sector(uint32_t sector_number, uint8_t* buff);
  uint8_t read_data(uint8_t* buff, uint32_t offset, uint32_t size);
  uint8_t write_data(uint8_t* buff, uint32_t offset, uint32_t size); 
  uint64_t get_sector_size();
};
