#include "ata_pio.h"
#include "../kheap.h"
#include <cstdint>

#define SECTOR_SIZE 512

uint64_t ata_pio_device::dispatch_command(ide_transaction transaction) {
  // handle register IO 
  if(transaction.exitinfo.port > 0x1F0 && transaction.exitinfo.port < 0x1F7 ||
     transaction.exitinfo.port > 0x170 && transaction.exitinfo.port < 0x177) {
      ata_pio_base_ports port = (ata_pio_base_ports)0;
      if(transaction.exitinfo.port > 0x1F0 && transaction.exitinfo.port < 0x1F7) {
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
  if(transaction.exitinfo.port == 0x1F7 || transaction.exitinfo.port == 0x177) {
    this->registers.cmd_reg = transaction.written_val;
    this->handle_command(transaction.written_val); 
  }
  
  // handle data register IO
  if(transaction.exitinfo.port == 0x1F0 || transaction.exitinfo.port == 0x170) {
    // write/read from data register, handle data IO
  }
}

void ata_pio_device::handle_command(uint16_t command) {
  if(command == 0x20) {
    // read 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = kmalloc(this->registers.sec_count_reg * this->storage_dev.get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev.get_sector_size();

  } else if(command == 0x30) {
    // write 28bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = kmalloc(this->registers.sec_count_reg * this->storage_dev.get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev.get_sector_size();

  } else if(command == 0x24) {
    // read 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = READ_SECTORS;
    this->transfer_buff = kmalloc(this->registers.sec_count_reg * this->storage_dev.get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev.get_sector_size();

  } else if(command == 0x34) {
    // write 48bit LBA
    this->device_in_transfer = 1;
    this->transfer_type = WRITE_SECTORS;
    this->transfer_buff = kmalloc(this->registers.sec_count_reg * this->storage_dev.get_sector_size());
    this->buff_offset = 0;
    this->size_remaining = this->registers.sec_count_reg * this->storage_dev.get_sector_size();
  }
}

void ata_pio_device::handle_read_data(ide_transaction transaction) {
  if(this->device_in_transfer == 1 && this->transfer_type == READ_SECTORS) {
    
  } else {
    // write to somewhere
  }
}

void ata_pio_device::handle_write_data(ide_transaction transaction) {
  if(this->device_in_transfer == 1 && this->transfer_type == WRITE_SECTORS) {

  } else {
    // write to somewhere
  }
}
