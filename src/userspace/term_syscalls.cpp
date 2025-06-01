#include "term_syscalls.h"
#include "../terminal.h"

void puts_syscall(const char *text) {
  terminal_puts(text);
}
