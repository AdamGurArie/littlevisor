#pragma once

#include <cstdint>

#define REGISTER_OPERANDS 0b11
#define MEM_OPERAND 0b00

enum operand_register {
  rax,
  rcx,
  rdx,
  rbx,
  sib_ah,
  disp32_ch,
  rsi,
  rdi
};

struct modrm {
  uint8_t rm : 3;
  uint8_t reg : 3;
  uint8_t mod : 2;
} __attribute__((packed));

struct operands_decoding {
  operand_register op1;
  operand_register op2;
  uint8_t only_regs;
};

operands_decoding decode_in(uint64_t rip);

operands_decoding decode_out(uint64_t rip);
