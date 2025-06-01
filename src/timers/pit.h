#pragma once

#include <cstdint>
void init_pit();
void pit_handler();
uint32_t get_ms_per_tick();
void sleep(uint64_t ms);
