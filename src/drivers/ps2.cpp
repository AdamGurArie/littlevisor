#include "ps2.h"
#include "acpi.h"
#include "../common.h"
#include "../interrupts.h"
#include <cstdint>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD 0x64

#define SCANCODE_SET_2 0x41

extern "C" void isr_kbd_handler();

uint8_t scan_codes[] = {
    0x1C, 0x32, 0x21, 0x23, 0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B, // A-J
    0x42, 0x4B, 0x3A, 0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C, // K-T
    0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A,                         // U-Z
    0x45, 0x16, 0x1E, 0x26, 0x25, 0x2E, 0x36, 0x3D, 0x3E, 0x46, // 0-9
    0x4E, 0x55, 0x5C,                                           // - = \ //
    0x66, 0x0D, 0x29,                                           // BKSP TAB SPACE
    0x76, 0x58, 0x5A,                                           // ESC CAPS ENTER
    0x12, 0x14, 0x11, 0x59,                                     // LSHIFT LCTRL LALT RSHIFT
    0x8C, 0x19, 0x8D, 0x8B,                                     // RCTRL RALT RWIN APPS
    0x67, 0x6E, 0x6F, 0x64, 0x65, 0x6D,                         // INS HOME PGUP DEL END PGDN
    0x75, 0x72, 0x6B, 0x74,                                     // ↑ ↓ ← →
    0x05, 0x06, 0x04, 0x0A, 0x07, 0x08, 0x09,                   // misc keys
    0x7C, 0x7B, 0x79, 0x7A, 0x6C, 0x70, 0x69, 0x72, 0x7D, 0x7E, // keypad
    0x71, 0x70, 0x7A, 0x6B, 0x73, 0x74, 0x6C, 0x75, 0x7C, 0x7B, // keypad cont.
    0x7D, 0x7E, 0x77, 0x7C, 0x7B, 0x79, 0x7A,                   // more keypad/Fn
    0x07, 0x0E, 0x09,                                           // more misc
    0x5E, 0x5F, 0x62                                            // scroll/pause
};

uint8_t keys[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '-', '=', '\\',
    0x08, 0x09, 0x20,             // BKSP, TAB, SPACE
    0x1B, 0x00, 0x0D,             // ESC, CAPS (0x00 = placeholder), ENTER
    0x00, 0x00, 0x00, 0x00,       // LSHIFT, LCTRL, LALT, RSHIFT
    0x00, 0x00, 0x00, 0x00,       // RCTRL, RALT, RWIN, APPS
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // INS, HOME, PGUP, DEL, END, PGDN
    0x00, 0x00, 0x00, 0x00,       // ↑ ↓ ← →
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // misc
    '*', '-', '+', '.', '/', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '+', '*', '-', '+',
    '/', '*', 0x00, '*', '-', '+', '/',  // keypad/Fn
    0x00, 0x00, 0x00                    // scroll, pause (as placeholders)
};

uint8_t get_scancode_set();
bool set_scancode_set3();
uint8_t find_key(uint8_t scan_code);
uint8_t read_ps2_byte();
void write_ps2_byte(uint8_t port, uint8_t data);

uint8_t read_ps2_byte() {
  while((read_from_port_byte(PS2_STATUS) & 1) == 0);
  return read_from_port_byte(PS2_DATA);
}

void write_ps2_byte(uint8_t port, uint8_t data) {
  while((read_from_port_byte(PS2_STATUS) & 2) == 1);
  write_to_port(port, data);
}

bool check_for_controller() {
  FADT* fadt = get_fadt();
  return getbit(fadt->boot_architecture_flags, 1);
}

void init_ps2_controller() {
  write_ps2_byte(PS2_CMD, 0xad);
  write_ps2_byte(PS2_CMD, 0xa7);

  // read_ps2_byte();

  write_ps2_byte(PS2_CMD, 0x20);
  uint8_t conf = read_ps2_byte();
  uint8_t aux_available = conf & 0x10;

  write_ps2_byte(PS2_DATA, 0xAA);
  if(read_ps2_byte() != 0x55) {
    return;
  }

  write_ps2_byte(PS2_DATA, 0xAE);
  /*conf |= 0x1;
  //conf |= 0x2;
  write_ps2_byte(PS2_DATA, 0x60);
  write_ps2_byte(PS2_CMD, conf);

  conf = (1 << 0) | (1 << 6);
  write_ps2_byte(PS2_CMD, 0x60);
  write_ps2_byte(PS2_DATA, conf);
  write_ps2_byte(PS2_CMD, 0xae);
  write_ps2_byte(PS2_CMD, 0xae);*/

  init_idt_entry(0x21, (uint64_t)isr_kbd_handler, 0, 0, 0xF);
  irq_clear_mask(0x1);
}

void handle_keyboard_int() {
  uint8_t scan_code = read_ps2_byte();
  if(scan_code == 0xF0) {
    read_ps2_byte();
    return;
  }
  
  uint8_t key = find_key(scan_code);
  // handle read key
}

uint8_t get_scancode_set() {
  write_ps2_byte(PS2_DATA, 0xF0);
  write_ps2_byte(PS2_DATA, 0);
  uint8_t resp = read_ps2_byte();
  if(resp != 0xFA) {
    return 0x0;
  }
  
  return read_ps2_byte();
}

bool set_scancode_set3() {
  write_ps2_byte(PS2_DATA, 0xF0);
  write_ps2_byte(PS2_DATA, 0x3);
  if(read_ps2_byte() == 0xFA) {
    return true; 
  }

  return false;
}

uint8_t find_key(uint8_t scan_code) {
  for(uint32_t i = 0; i < sizeof(scan_codes); i++) {
    if(scan_codes[i] == scan_code) {
      return keys[i];
    }
  }

  return 0;
}
