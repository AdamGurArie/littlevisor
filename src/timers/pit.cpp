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
uint32_t time_between_ticks_ms = 0;
uint32_t ticks_in_sec = 0;
uint32_t frequency = 0;

void init_pit() {
  write_to_port(PIT_COMMANDS, (uint8_t)0x36);

  //time_between_ticks_ms = FREQUENCY * 3000 / 3579545;
  //ticks_in_sec = 1000 / time_between_ticks_ms;
  uint32_t divisor = 1193182 / FREQUENCY;
  write_to_port(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));
  write_to_port(PIT_CHANNEL_0, (uint8_t)((divisor >> 8) & 0xFF));
  frequency = FREQUENCY;
  //frequency = (1193182 + (divisor / 2)) / divisor;
  time_between_ticks_ms = 1000  / frequency;
  init_idt_entry(0x20, (uint64_t)isr_pit_handler, 0, 0, 0xF);
  irq_clear_mask(0x0);
}

void pit_handler(Stack* stack) {
  tick_counter++;
  round_robin(stack);
}

void sleep(uint64_t ms) {
  uint64_t finale_tick_count = tick_counter + ms / time_between_ticks_ms;
  while(tick_counter < finale_tick_count);
}

uint32_t get_ms_per_tick() {
  return time_between_ticks_ms;
}
