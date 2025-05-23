#include "ata_pio.h"
#include "../kheap.h"
#include "../common.h"
#include "../vmcs/vmcs.h"
#include "../fs/vfs.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#define SECTOR_SIZE 512

static void construct_response(ata_response* response, uint8_t* data, uint32_t size);

void construct_response(ata_response* response, uint8_t* data, uint32_t size) {
  if(size == 0 || data == NULL) {
    // kdebug()?;
    return;
  }

  response->buffer = (uint8_t*)kmalloc(size);
  response->size = size;
  kmemcpy((uint8_t*)response->buffer, data, size);
  return;
}

uint16_t ata_pio_device::handle_read_register(ata_pio_base_ports port) {
  switch(port) {
    case DATA_REGISTER:
      return this->registers.data_reg;

    case ERROR_FEATURES_REGISTER:
      return this->registers.error_reg;

    case SECTOR_COUNT_REGISTER:
      return this->registers.sec_count_reg;

    case SECTOR_NUMBER_REGISTER_LBALOW:
      return this->registers.sec_num_reg & 0xFF;

    case CYLINDER_LOW_REGISTER_LBAMID:
      return (this->registers.sec_num_reg << 8) & 0xFF;

    case CYLINDER_HIGH_REGISTER_LBAHIGH:
      return (this->registers.cylinder_high_reg) & 0xFFFF;

    case DRIVE_HEAD_REGISTER:
      return this->registers.drive_head_reg;

    case STATUS_COMMAND_REGISTER:
      return this->registers.status_reg;

    case ALT_STATUS_REG_DEV_CTRL:
      return this->registers.alt_status_reg;

    case DRIVE_ADDR_REG:
      return this->registers.drive_addr_reg;

    default:
      return 0;
  }

  return 0;
}

void ata_pio_device::handle_write_register(ata_pio_base_ports port, uint16_t data) {
  switch(port) {
    case DATA_REGISTER:
      this->registers.data_reg = data;
      return;

    case ERROR_FEATURES_REGISTER:
      this->registers.features_reg = data;
      return;

    case SECTOR_COUNT_REGISTER:
      this->registers.sec_count_reg = data;
      return;

    case SECTOR_NUMBER_REGISTER_LBALOW:
      this->registers.sec_num_reg &= 0xFF00;
      this->registers.sec_num_reg = this->registers.sec_num_reg | (data & 0xFF);
      return;

    case CYLINDER_LOW_REGISTER_LBAMID:
      this->registers.sec_num_reg &= 0xFF00;
      this->registers.sec_num_reg = this->registers.sec_num_reg | (data & 0xFF);
      return;

    case CYLINDER_HIGH_REGISTER_LBAHIGH:
      this->registers.cylinder_high_reg &= 0xFF00;
      this->registers.cylinder_high_reg = this->registers.cylinder_high_reg | (data & 0xFFFF);
      return;

    case DRIVE_HEAD_REGISTER:
      this->registers.drive_head_reg = data;
      return;

    case STATUS_COMMAND_REGISTER:
      this->registers.cmd_reg = data;
      return;

    case ALT_STATUS_REG_DEV_CTRL:
      this->registers.dev_ctrl_reg = data & 0xFF;

    default:
      return;
  }
}

void ata_pio_device::dispatch_command(ide_transaction transaction, ata_response* response) {
  // handle register IO 
  if(((transaction.exitinfo.port > 0x1F0) && (transaction.exitinfo.port < 0x1F7)) ||
     ((transaction.exitinfo.port > 0x170) && (transaction.exitinfo.port < 0x177))) {
      ata_pio_base_ports port = (ata_pio_base_ports)0;
      if((transaction.exitinfo.port > 0x1F0) && (transaction.exitinfo.port < 0x1F7)) {
        port = (ata_pio_base_ports)(transaction.exitinfo.port - 0x1F0);
      } else {
        port = (ata_pio_base_ports)(transaction.exitinfo.port - 0x170);
      }

      if(transaction.exitinfo.type == 0) {
        // write to register
        this->handle_write_register(port, transaction.written_val);
      } else {
        // read from register
        uint64_t ret_val = this->handle_read_register(port);
        construct_response(response, (uint8_t*)&ret_val, sizeof(ret_val));
        //edit_vmcb_state(RAX, ret_val);
      }
  }

  // handle commands and status
  if((transaction.exitinfo.port == 0x1F7) || (transaction.exitinfo.port == 0x177)) {
    if(transaction.exitinfo.type == 1) {
      uint64_t ret_val = this->handle_read_register(STATUS_COMMAND_REGISTER);
      construct_response(response, (uint8_t*)&ret_val, sizeof(ret_val));
      // edit_vmcb_state(RAX, ret_val);

    } else {
      this->registers.cmd_reg = transaction.written_val;
      this->handle_command(transaction.written_val);

    }

  } 

  if((transaction.exitinfo.port == 0x1F0) || (transaction.exitinfo.port == 0x170)) {
    // write/read from data register, handle data IO
    if(transaction.exitinfo.type == 0) {
      this->handle_write_data(transaction);

    } else {
      this->handle_read_data(transaction, response);

    }
  }

  if(transaction.exitinfo.port == ALT_STATUS_REG_DEV_CTRL) {
    if(transaction.exitinfo.type == 0) {
      this->handle_write_register((ata_pio_base_ports)transaction.exitinfo.port, transaction.written_val);
      this->handle_device_ctrl_reg_write();

    } else {
      this->handle_read_data(transaction, response);

    }
  }
}

void ata_pio_device::handle_device_ctrl_reg_write() {
  if(getbit(this->registers.dev_ctrl_reg, DEV_CTRL_SRST)) {
    this->handle_device_reset();
  }

  if(getbit(this->registers.dev_ctrl_reg, DEV_CTRL_NIEN)) {
    // handle stop sending interrupts
  }
}

void ata_pio_device::handle_device_reset() {
  this->registers.alt_status_reg = 0;
  this->registers.sec_count_reg = 0;
  this->registers.cmd_reg = 0;
  this->registers.data_reg = 0;
  this->registers.error_reg = 0;
  this->registers.status_reg = 0;
  this->registers.features_reg = 0;
  this->registers.dev_ctrl_reg = 0;
  this->registers.drive_addr_reg = 0;
  this->registers.drive_head_reg = 0;
  this->registers.cylinder_high_reg = 0;
  this->registers.sec_num_reg = 0;

  // set status to ready
  setbit(&this->registers.status_reg, STATUS_REG_RDY);
}

void ata_pio_device::handle_command(uint16_t command) {
  if(command == 0x20) {
    // read 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    kmemset((uint8_t*)this->transfer_buff, 0x0, this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_read_sectors();
    clearbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
    setbit(&this->registers.status_reg, 3);

  } else if(command == 0x30) {
    // write 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    kmemset((uint8_t*)this->transfer_buff, 0x0, this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    clearbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
    setbit(&this->registers.status_reg, 3);

  } else if(command == 0x24) {
    // read 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    kmemset((uint8_t*)this->transfer_buff, 0x0, this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_read_sectors();
    clearbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
    setbit(&this->registers.status_reg, 3);

  } else if(command == 0x34) {
    // write 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    kmemset((uint8_t*)this->transfer_buff, 0x0, this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_write_sectors();
    clearbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
    setbit(&this->registers.status_reg, 3);

  } else if(command == 0xEC) {
    // identify command
    this->device_in_transfer = 1;
    this->transfer_type = IDENTIFY_CMD;
    this->transfer_buff = (uint8_t*)kmalloc(512);
    kmemset((uint8_t*)this->transfer_buff, 0x0, this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = 512;
    this->handle_identify_command();
    clearbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
    setbit(&this->registers.status_reg, 3);

  } else {
    setbit(&this->registers.status_reg, 0);
    setbit(&this->registers.status_reg, 6);
  }
}

void ata_pio_device::handle_read_data(ide_transaction transaction, ata_response* response) {
  if(this->device_in_transfer == 1 &&
    (this->transfer_type == READ_SECTORS || this->transfer_type == IDENTIFY_CMD)) {

    if(transaction.exitinfo.sz8) {
      uint8_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint8_t));
      construct_response(response, (uint8_t*)&data, sizeof(data));
      this->buff_offset += sizeof(uint8_t);

    } else if(transaction.exitinfo.sz16) {
      uint16_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint16_t));
      construct_response(response, (uint8_t*)&data, sizeof(data));
      // edit_vmcb_state(RAX, data);
      this->buff_offset += sizeof(uint16_t);

    } else if(transaction.exitinfo.sz32) {
      uint32_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint32_t));
      construct_response(response, (uint8_t*)&data, sizeof(data));
      // edit_vmcb_state(RAX, data);
      this->buff_offset += sizeof(uint32_t);

    }

  } else {
    edit_vmcb_state(RAX, 0);
  }

  if(this->buff_offset == this->size_remaining) {
    this->device_in_transfer = 0;
    clearbit(&this->registers.status_reg, STATUS_REG_DRQ);
  } else {
    // notify that new data had arrived
  }
}

void ata_pio_device::handle_write_data(ide_transaction transaction) {
  if(this->device_in_transfer == 1 && this->transfer_type == WRITE_SECTORS) {
    if(transaction.exitinfo.sz8) {
      kmemcpy((uint8_t*)&this->transfer_buff[this->buff_offset], (uint8_t*)&transaction.written_val, sizeof(uint8_t));
      this->buff_offset += sizeof(uint8_t);

    } else if(transaction.exitinfo.sz16) {
      kmemcpy((uint8_t*)&this->transfer_buff[this->buff_offset], (uint8_t*)&transaction.written_val, sizeof(uint16_t));
      this->buff_offset += sizeof(uint16_t);

    } else if(transaction.exitinfo.sz32) {
      kmemcpy((uint8_t*)&this->transfer_buff[this->buff_offset], (uint8_t*)&transaction.written_val, sizeof(uint32_t));
      this->buff_offset += sizeof(uint32_t);

    }
  } else {
    // write to somewhere
  }

  if(this->buff_offset == this->size_remaining) {
    this->device_in_transfer = 0;
    this->handle_write_sectors();
  }
}

void ata_pio_device::handle_identify_command() {
  kmemset(this->transfer_buff, 0xff, 512);
  //kmemset(&this->transfer_buff[100], 0x10, 1);
  //kmemset(&this->transfer_buff[60], 0x10, 2);
  setbit(&this->registers.status_reg, STATUS_REG_DRQ);
}

void ata_pio_device::handle_read_sectors() {
  uint32_t offset = 0;
  uint32_t sector = this->registers.sec_num_reg;
  uint32_t sector_size = this->storage_dev->get_sector_size();

  for(uint32_t i = 0; i <= this->registers.sec_num_reg; i++) {
    storage_dev->read_sector(sector + i, &this->transfer_buff[offset]);
    offset += sector_size;
  }
}

void ata_pio_device::handle_write_sectors() {
  uint32_t offset = 0;
  uint32_t sector = this->registers.sec_num_reg;
  uint32_t sector_size = this->storage_dev->get_sector_size();

  for(uint32_t i = 0; i < this->registers.sec_num_reg; i++) {
    storage_dev->write_sector(sector + i, &this->transfer_buff[offset]);
    offset += sector_size;
  }
}

uint8_t virtual_storage_device::read_sector(uint32_t sector_number, uint8_t* buff) {
  uint32_t fd = vopenFile(this->file_name);
  vseekp(fd, sector_number * this->sector_size);
  vreadFile(fd, (char*)buff, this->sector_size);
  return 0;
}

uint8_t virtual_storage_device::write_sector(uint32_t sector_number, uint8_t* buff) {
  uint32_t fd = vopenFile(this->file_name);
  vseekp(fd, sector_number * this->sector_size);
  vreadFile(fd, (char*)buff, this->sector_size);
  return 0;
}

uint8_t virtual_storage_device::read_data(uint8_t* buff, uint32_t offset, uint32_t size) {
  uint32_t fd = vopenFile(this->file_name);
  vseekp(fd, offset);
  vreadFile(fd, (char*)buff, size);
  return 0;
}

uint8_t virtual_storage_device::write_data(uint8_t* buff, uint32_t offset, uint32_t size) {
  uint32_t fd = vopenFile(this->file_name);
  vseekp(fd, offset);
  vreadFile(fd, (char*)buff, size);
  return 0;
}

uint64_t virtual_storage_device::get_sector_size() {
  return this->sector_size;
}

/* uint64_t build_identify_cmd() {
  // will might have problems with endians
  uint16_t identify_cmd[256];
  kmemset((uint8_t*)identify_cmd, 0x0, 256);
  identify_cmd[0] = 0x040;
  identify_cmd[1] = 16383; // number of cylinders 
  identify_cmd[3] = 0;     // number of heads
} */
