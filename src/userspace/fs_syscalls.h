#pragma once

#include <cstdint>
#include <sys/types.h>
uint32_t open_file_syscall(char* filename);
void read_file_syscall(uint32_t file_handle, uint8_t* buff, uint32_t size);
void write_file_syscall(uint32_t file_handle, uint8_t* buff, uint32_t size);
void seek_file_syscall(uint32_t file_handle, uint32_t pos);
void close_file_syscall(uint32_t file_handle);
