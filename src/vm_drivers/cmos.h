#pragma once

#include <cstdint>
#include <stdint.h>

enum cmos_index {
  CMOS_BIOS_BOOTFLAG1 = 0x38,
  CMOS_BIOS_BOOTFLAG2 = 0x39
};

class cmos_device {
private:
  uint16_t index_reg;
  uint8_t bootflag1;
  uint8_t bootflag2;

public:
  cmos_device() {
    this->index_reg = 0;
    this->bootflag1 = (1 << 4) | 0;
    this->bootflag2 = (3 << 4) | (2 << 0);
  }

  void write_reg_idx(cmos_index reg_idx);
  void write_reg(uint8_t val);
  uint8_t read_reg();
};
