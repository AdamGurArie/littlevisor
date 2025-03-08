#include "cmos.h"
#include <cstdint>

void cmos_device::write_reg_idx(cmos_index reg_idx) {
  this->index_reg = reg_idx;
}

uint8_t cmos_device::read_reg() {
  switch(this->index_reg) {
    case CMOS_BIOS_BOOTFLAG1:
      return this->bootflag1;

    case CMOS_BIOS_BOOTFLAG2:
      return this->bootflag2;

    default:
      return 0;
  }
}

void cmos_device::write_reg(uint8_t val) {
  switch(this->index_reg) {
    case CMOS_BIOS_BOOTFLAG1:
      this->bootflag1 = val;

    case CMOS_BIOS_BOOTFLAG2:
      this->bootflag2 = val;
  }
}
