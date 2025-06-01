#ifndef FLANTERM_FB_H
#define FLANTERM_FB_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../flanterm.h"

#ifdef FLANTERM_IN_FLANTERM

#include "fb_private.h"

#endif

struct flanterm_context *flanterm_fb_init(
    /* If _malloc and _free are nulled, use the bump allocated instance (1 use only). */
    void *(*_malloc)(size_t size),
    void (*_free)(void *ptr, size_t size),
    uint32_t *framebuffer, size_t width, size_t height, size_t pitch,
    uint8_t red_mask_size, uint8_t red_mask_shift,
    uint8_t green_mask_size, uint8_t green_mask_shift,
    uint8_t blue_mask_size, uint8_t blue_mask_shift,
    uint32_t *canvas, /* If nulled, no canvas. */
    uint32_t *ansi_colours, uint32_t *ansi_bright_colours, /* If nulled, default. */
    uint32_t *default_bg, uint32_t *default_fg, /* If nulled, default. */
    uint32_t *default_bg_bright, uint32_t *default_fg_bright, /* If nulled, default. */
    /* If font is null, use default font and font_width and font_height ignored. */
    void *font, size_t font_width, size_t font_height, size_t font_spacing,
    /* If scale_x and scale_y are 0, automatically scale font based on resolution. */
    size_t font_scale_x, size_t font_scale_y,
    size_t margin
);

#ifdef __cplusplus
}
#endif

#endif
