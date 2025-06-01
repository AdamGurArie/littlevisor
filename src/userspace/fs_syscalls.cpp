#include "fs_syscalls.h"
#include "../fs/vfs.h"
#include <cstdint>

uint32_t open_file_syscall(char *filename) {
  return vopenFile(filename);
}

void read_file_syscall(uint32_t file_handle, uint8_t* buff, uint32_t size) {
  vreadFile(file_handle, (char*)buff, size);
}

void write_file_syscall(uint32_t file_handle, uint8_t* buff, uint32_t size) {
  vwriteFile(file_handle, (char*)buff, size);
}

void seek_file_syscall(uint32_t file_handle, uint32_t pos) {
  vseekp(file_handle, pos);
}

void close_file_syscall(uint32_t file_handle) {

}
