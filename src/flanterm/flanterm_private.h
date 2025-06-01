#ifndef FLANTERM_PRIVATE_H
#define FLANTERM_PRIVATE_H 1

#ifndef FLANTERM_IN_FLANTERM
#error "Do not use flanterm_private.h. Use interfaces defined in flanterm.h only."
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLANTERM_MAX_ESC_VALUES 16

struct flanterm_context {
    /* internal use */

    size_t tab_size;
    bool autoflush;
    bool cursor_enabled;
    bool scroll_enabled;
    bool control_sequence;
    bool escape;
    bool osc;
    bool osc_escape;
    bool rrr;
    bool discard_next;
    bool bold;
    bool bg_bold;
    bool reverse_video;
    bool dec_private;
    bool insert_mode;
    uint64_t code_point;
    size_t unicode_remaining;
    uint8_t g_select;
    uint8_t charsets[2];
    size_t current_charset;
    size_t escape_offset;
    size_t esc_values_i;
    size_t saved_cursor_x;
    size_t saved_cursor_y;
    size_t current_primary;
    size_t current_bg;
    size_t scroll_top_margin;
    size_t scroll_bottom_margin;
    uint32_t esc_values[FLANTERM_MAX_ESC_VALUES];
    uint64_t oob_output;
    bool saved_state_bold;
    bool saved_state_bg_bold;
    bool saved_state_reverse_video;
    size_t saved_state_current_charset;
    size_t saved_state_current_primary;
    size_t saved_state_current_bg;

    /* to be set by backend */

    size_t rows, cols;

    void (*raw_putchar)(struct flanterm_context *, uint8_t c);
    void (*clear)(struct flanterm_context *, bool move);
    void (*set_cursor_pos)(struct flanterm_context *, size_t x, size_t y);
    void (*get_cursor_pos)(struct flanterm_context *, size_t *x, size_t *y);
    void (*set_text_fg)(struct flanterm_context *, size_t fg);
    void (*set_text_bg)(struct flanterm_context *, size_t bg);
    void (*set_text_fg_bright)(struct flanterm_context *, size_t fg);
    void (*set_text_bg_bright)(struct flanterm_context *, size_t bg);
    void (*set_text_fg_rgb)(struct flanterm_context *, uint32_t fg);
    void (*set_text_bg_rgb)(struct flanterm_context *, uint32_t bg);
    void (*set_text_fg_default)(struct flanterm_context *);
    void (*set_text_bg_default)(struct flanterm_context *);
    void (*set_text_fg_default_bright)(struct flanterm_context *);
    void (*set_text_bg_default_bright)(struct flanterm_context *);
    void (*move_character)(struct flanterm_context *, size_t new_x, size_t new_y, size_t old_x, size_t old_y);
    void (*scroll)(struct flanterm_context *);
    void (*revscroll)(struct flanterm_context *);
    void (*swap_palette)(struct flanterm_context *);
    void (*save_state)(struct flanterm_context *);
    void (*restore_state)(struct flanterm_context *);
    void (*double_buffer_flush)(struct flanterm_context *);
    void (*full_refresh)(struct flanterm_context *);
    void (*deinit)(struct flanterm_context *, void (*)(void *, size_t));

    /* to be set by client */

    void (*callback)(struct flanterm_context *, uint64_t, uint64_t, uint64_t, uint64_t);
};

void flanterm_context_reinit(struct flanterm_context *ctx);

#ifdef __cplusplus
}
#endif

#endif
