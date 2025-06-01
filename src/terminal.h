#pragma once
#include "limine.h"

void init_terminal(limine_framebuffer* framebuffer);
void terminal_puts(const char* string);
void terminal_flush();
void terminal_refresh();
