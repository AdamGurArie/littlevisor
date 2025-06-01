#include "flanterm/flanterm.h"
#include "flanterm/backends/fb.h"
#include "limine.h"
#include "common.h"
#include "mm/npaging.h"
#include <cstdint>
#include <cstring>
#include <type_traits>

struct flanterm_context* ctx = 0;
bool is_initialized = false;
uint64_t terminal_pagemap = 0; // @TODO: DIRTY HACK!!!! FIX IT!


void init_terminal(limine_framebuffer* framebuffer) {
  mapPage(TO_LOWER_HALF((uint64_t)framebuffer), (uint64_t)framebuffer, 0x03, 0);
  mapPage(TO_LOWER_HALF((uint64_t)framebuffer->address), (uint64_t)framebuffer->address, 0x03, 0);
  ctx = flanterm_fb_init(NULL,
      NULL,
      (uint32_t*)framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,
      framebuffer->red_mask_size, framebuffer->red_mask_shift, framebuffer->green_mask_size,
      framebuffer->green_mask_shift, framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
      NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL,
      0, 0, 1, 0, 0, 0);

  is_initialized = true;

  terminal_pagemap = get_host_pageMap();
}

void terminal_puts(const char* string) {
  uint32_t len = kstrlen(string);
  flanterm_write(ctx, string, len);
}

void terminal_flush() {
  flanterm_flush(ctx);
}

void terminal_refresh() {
  flanterm_full_refresh(ctx);
}
