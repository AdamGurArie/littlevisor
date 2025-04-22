#include "pit.h"
#include "../common.h"
#include <cstdint>
#include "../interrupts.h"
#include "../scheduler/scheduler.h"

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMANDS 0x43

#define FREQUENCY 200

extern "C" void isr_pit_handler();

uint64_t tick_counter = 0;

void init_pit() {
  write_to_port(PIT_COMMANDS, (uint8_t)0x36);

  uint32_t divisor = 1193182 / FREQUENCY;
  write_to_port(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));
  write_to_port(PIT_CHANNEL_1, (uint8_t)((divisor >> 8) & 0xFF));

  init_idt_entry(0x20, (uint64_t)isr_pit_handler, 0, 0, 0xF);
  irq_clear_mask(0x0);
}

void pit_handler(Stack* stack) {
  tick_counter++;
  round_robin(stack);
}
