#include "instruction_decoder.h"
#include <cstdint>
#include <bit>

#define in_op  0xED
#define out_op 0xEF

operands_decoding decode_in(uint64_t rip) {
  uint8_t* inst = (uint8_t*)rip;
  modrm* mrm = std::bit_cast<modrm*>(&inst[1]);
  operands_decoding decode = {};
  decode.op1 = (operand_register)mrm->reg;
  decode.op2 = (operand_register)mrm->rm;
  if(mrm->mod == 0b11) {
    decode.only_regs = 1;
  } else {
    decode.only_regs = 0;
  }

  return decode;
}

operands_decoding decode_out(uint64_t rip) {

}
