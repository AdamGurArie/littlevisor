#include "ide.h"

#include "../mm/npaging.h"
#include <cstdint>

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

}

void ideDevice::handle_writeSector() {

}

uint64_t ideDevice::handle_registerRead(ide_transaction transaction) {
  if(transaction.exitinfo.port == 0x1F0 || transaction.exitinfo.port == 0x1F1 ||
     transaction.exitinfo.port == 0x1F2 || transaction.exitinfo.port == 0x1F3 || transaction.exitinfo.port == 0x1F) {
    
    return this->registers[transaction.exitinfo.port - 0x1F0];
  } else if(transaction.exitinfo.port == 0x3F6 || transaction.exitinfo.port == 0x3F7) {
    return this->registers[transaction.exitinfo.port - 0x3F6];
  }

  return 0;
}

void ideDevice::handle_registerWrite(ide_transaction transaction) {
  if(transaction.exitinfo.port == 0x1F0 || transaction.exitinfo.port == 0x1F1 || transaction.exitinfo.port == 0x1F2 || transaction.exitinfo.port == 0x1F3 ||
     transaction.exitinfo.port == 0x1F4 || transaction.exitinfo.port == 0x1F5 || transaction.exitinfo.port == 0x1F6 || transaction.exitinfo.port == 0x1F7) {

    this->registers[transaction.exitinfo.port - 0x1F0] = transaction.written_val;
  } else if(transaction.exitinfo.port == 0x3F7) {
    this->registers[transaction.exitinfo.port - 0x3F6] = transaction.written_val;
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
}
