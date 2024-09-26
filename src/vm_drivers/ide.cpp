/**#include "ide.h"

#include "../mm/npaging.h"
#include "../fs/vfs.h"
#include <cstdint>
#include <cstring>

void setup_virtualized_ide() {
  mapPage(IO_MASTER_BASE_PORT, IO_MASTER_BASE_PORT, 0x0);
  mapPage(IO_MASTER_BASE_CONTROL, IO_MASTER_BASE_CONTROL, 0x0);
  mapPage(IO_SLAVE_BASE_PORT, IO_SLAVE_BASE_PORT, 0x0);
  mapPage(IO_SLAVE_CONTROL, IO_SLAVE_CONTROL, 0x0);
  mapPage(0xb8000, 0xb8000, 0x07);
}

uint64_t ideDevice::handle_transaction(ide_transaction transaction) {
  if(this->virtualized == true) {
    return passthrough_transaction(transaction);
  }

  if(transaction.exitinfo.type == 0) {
    return this->handle_registerRead(transaction);
  } else {
    handle_registerWrite(transaction);
    return 0;
  }
}

void ideDevice::handle_readSector() {
  uint64_t sector = sector_number_register_lbalow;
  sector |= cylinder_low_register_lbamid >> 16;
  sector |= (uint64_t)cylinder_high_register_lbahigh >> 32;
  uint8_t buff[SECTOR_SIZE*sector_count_register];

  memset((char*)buff, 0x0, sizeof(buff));
  seekp(fd, sector*SECTOR_SIZE);
  readFile(fd, (char*)buff, sector_count_register*SECTOR_SIZE);

  // buffer_with_data = buff;
  // size_of_data_to_transfer = sector_count_register*SECTOR_SIZE;
}

uint8_t ideDevice::check_if_ready_for_transfer() {
  return this->ready_to_trasnfer; 
}

void ideDevice::handle_writeSector() {

}

uint16_t ideDevice::read_data_io() {
  if(offset_in_data >= size_of_data_to_transfer) {
    return 0;
  }

  uint16_t value = (uint16_t)*(buffer_with_data + offset_in_data);
  offset_in_data += 2;
  return value;
}

void ideDevice::write_data_io(uint16_t data) {
  if(offset_in_data >= size_of_data_to_transfer) {
    return;
  }

  memcpy((char*)buffer_with_data, (char*)&data, sizeof(data));
}

uint64_t ideDevice::handle_registerRead(ide_transaction transaction) {
  if(transaction.exitinfo.port == 0x1F0 || transaction.exitinfo.port == 0x1F1 ||
     transaction.exitinfo.port == 0x1F2 || transaction.exitinfo.port == 0x1F3 || transaction.exitinfo.port == 0x1F) {
    
    return this->registers[transaction.exitinfo.port - 0x1F0];
  } else if(transaction.exitinfo.port == 0x3F6 || transaction.exitinfo.port == 0x3F7) {
    return this->registers[transaction.exitinfo.port - 0x3F6];
  } else if(transaction.exitinfo.port == 0x1F0) {
    this->ready_to_trasnfer = 1;
    return read_data_io(); 
  }

  return 0;
}

void ideDevice::handle_registerWrite(ide_transaction transaction) {
  if(transaction.exitinfo.port == 0x1F0 || transaction.exitinfo.port == 0x1F1 || transaction.exitinfo.port == 0x1F2 || transaction.exitinfo.port == 0x1F3 ||
     transaction.exitinfo.port == 0x1F4 || transaction.exitinfo.port == 0x1F5 || transaction.exitinfo.port == 0x1F6 || transaction.exitinfo.port == 0x1F7) {

    this->registers[transaction.exitinfo.port - 0x1F0] = transaction.written_val;
  } else if(transaction.exitinfo.port == 0x3F7) {
    this->registers[transaction.exitinfo.port - 0x3F6] = transaction.written_val;
  } else if(transaction.exitinfo.port == 0x1F0) {
    this->write_data_io(transaction.written_val);
  }
}

uint64_t ideDevice::passthrough_transaction(ide_transaction transaction) {
  if(transaction.exitinfo.type == 0) {
    // in
    uint64_t read_value = 0;
    //asm volatile("in %0" : "=r"(read_value) : "r"(transaction.exitinfo.port));
    return read_value;
  } else {
    // out
    //asm volatile("mov %%rax, %0" :: "r"(transaction.written_val));
    //asm volatile("out %0" :: "r"(transaction.exitinfo.port));
    return 0;
  }
}**/
