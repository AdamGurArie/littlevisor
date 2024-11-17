#include "ata_pio.h"
#include "../kheap.h"
#include "../common.h"
#include "../vmcs/vmcs.h"
#include "../fs/vfs.h"
#include <cstdint>
#include <cstdlib>

#define SECTOR_SIZE 512

uint16_t ata_pio_device::handle_read_register(ata_pio_base_ports port) {
  switch(port) {
    case DATA_REGISTER:
      return this->registers.data_reg;

    case ERROR_REGISTER:
      return this->registers.error_reg;

    case FEATURES_REGISTER:
      return this->registers.features_reg;

    case SECTOR_COUNT_REGISTER:
      return this->registers.sec_count_reg;

    case SECTOR_NUMBER_REGISTER_LBALOW:
      return this->registers.sec_num_reg & 0xFFFF;

    case CYLINDER_LOW_REGISTER_LBAMID:
      return (this->registers.sec_num_reg << 16) & 0xFFFF;

    case CYLINDER_HIGH_REGISTER_LBAHIGH:
      return (this->registers.sec_num_reg << 24) & 0xFFFF;

    case DRIVE_HEAD_REGISTER:
      return this->registers.drive_head_reg;

    case STATUS_REGISTER:
      return this->registers.status_reg;

    case COMMAND_REGISTER:
      return this->registers.cmd_reg;
  }

  return 0;
}

void ata_pio_device::handle_write_register(ata_pio_base_ports port, uint16_t data) {
  switch(port) {
    case DATA_REGISTER:
      this->registers.data_reg = data;
      return;

    case ERROR_REGISTER:
      this->registers.error_reg = data;
      return;

    case FEATURES_REGISTER:
      this->registers.features_reg = data;
      return;

    case SECTOR_COUNT_REGISTER:
      this->registers.sec_count_reg = data;
      return;

    case SECTOR_NUMBER_REGISTER_LBALOW:
      this->registers.sec_num_reg = this->registers.sec_num_reg | (data & 0xFFFF);
      return;

    case CYLINDER_LOW_REGISTER_LBAMID:
      this->registers.sec_num_reg = this->registers.sec_num_reg | ((data << 16) & 0xFFFF);
      return;

    case CYLINDER_HIGH_REGISTER_LBAHIGH:
      this->registers.sec_num_reg = this->registers.sec_num_reg | ((data << 24) & 0xFFFF);
      return;

    case DRIVE_HEAD_REGISTER:
      this->registers.drive_head_reg = data;
      return;

    case STATUS_REGISTER:
      this->registers.status_reg = data;
      return;

    case COMMAND_REGISTER:
      this->registers.cmd_reg = data;
      return;
  }
}

void ata_pio_device::dispatch_command(ide_transaction transaction) {
  // handle register IO 
  if(((transaction.exitinfo.port > 0x1F0) && (transaction.exitinfo.port < 0x1F7)) ||
     ((transaction.exitinfo.port > 0x170) && (transaction.exitinfo.port < 0x177))) {
      ata_pio_base_ports port = (ata_pio_base_ports)0;
      if((transaction.exitinfo.port > 0x1F0) && (transaction.exitinfo.port < 0x1F7)) {
        port = (ata_pio_base_ports)(transaction.exitinfo.port - 0x1F0);
      } else {
        port = (ata_pio_base_ports)(transaction.exitinfo.port - 0x170);
      }

      if(transaction.exitinfo.type == 1) {
        // write to register
        this->handle_write_register(port, transaction.written_val);
      } else {
        // read from register
        this->handle_read_register(port);
      }
  }

  // handle commands
  if((transaction.exitinfo.port == 0x1F7) || (transaction.exitinfo.port == 0x177)) {
    this->registers.cmd_reg = transaction.written_val;
    this->handle_command(transaction.written_val); 
  }
  
  // handle data register IO
  if((transaction.exitinfo.port == 0x1F0) || (transaction.exitinfo.port == 0x170)) {
    // write/read from data register, handle data IO
    if(transaction.exitinfo.type == 1) {
      this->handle_write_data(transaction);

    } else {
      this->handle_read_data(transaction);

    }
  }
}

void ata_pio_device::handle_command(uint16_t command) {
  if(command == 0x20) {
    // read 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_read_sectors();

  } else if(command == 0x30) {
    // write 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();

  } else if(command == 0x24) {
    // read 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_read_sectors();

  } else if(command == 0x34) {
    // write 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = (uint8_t*)kmalloc(this->registers.sec_count_reg * this->storage_dev->get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev->get_sector_size();
    this->handle_write_sectors();

  } else if(command == 0xEC) {
    // identify command
    this->device_in_transfer = 1;
    this->transfer_type = IDENTIFY_CMD;
    this->transfer_buff = (uint8_t*)kmalloc(512);
    this->buff_offset = 0;
    this->size_remaining = 512;
    this->handle_identify_command();

  }
}

void ata_pio_device::handle_read_data(ide_transaction transaction) {
  if(this->device_in_transfer == 1 && this->transfer_type == READ_SECTORS) {
    if(transaction.exitinfo.sz8) {
      uint8_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint8_t));
      edit_vmcb_state(RAX, data);
      this->buff_offset += sizeof(uint8_t);

    } else if(transaction.exitinfo.sz16) {
      uint16_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint16_t));
      edit_vmcb_state(RAX, data);
      this->buff_offset += sizeof(uint16_t);

    } else if(transaction.exitinfo.sz32) {
      uint32_t data = 0;
      kmemcpy((uint8_t*)&data, (uint8_t*)&this->transfer_buff[this->buff_offset], sizeof(uint32_t));
      edit_vmcb_state(RAX, data);
      this->buff_offset += sizeof(uint32_t);

    }

  } else {
    edit_vmcb_state(RAX, 0);
  }

  if(this->buff_offset == this->size_remaining) {
    this->device_in_transfer = 0;
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
  kmemset(this->transfer_buff, 0x0, 512);
}

void ata_pio_device::handle_read_sectors() {
  uint32_t offset = 0;
  uint32_t sector = this->registers.sec_num_reg;
  uint32_t sector_size = this->storage_dev->get_sector_size();

  for(uint32_t i = 0; i < this->registers.sec_num_reg; i++) {
    storage_dev->read_sector(&this->transfer_buff[offset], sector + i, 1);
    offset += sector_size;
  }
}

void ata_pio_device::handle_write_sectors() {
  uint32_t offset = 0;
  uint32_t sector = this->registers.sec_num_reg;
  uint32_t sector_size = this->storage_dev->get_sector_size();

  for(uint32_t i = 0; i < this->registers.sec_num_reg; i++) {
    storage_dev->write_sector(&this->transfer_buff[offset], sector + i, 1);
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
