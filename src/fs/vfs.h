#pragma once

#include "fat32.h"
#include <cstdint>

#define OPEN_FILE_ERROR 101

struct VFILE_DESCRIPTOR {
  char filename[12];
  uint32_t position;
  uint8_t occupied;
};

uint32_t vopenFile(const char* filename);
uint32_t vreadFile(uint32_t fd, char* buff, uint32_t size);
uint32_t vwriteFile(uint32_t fd, char* buff, uint32_t size);
uint32_t vseekp(uint32_t fd, uint32_t new_pos);
uint32_t vseekr(uint32_t fd, uint32_t forward);
uint32_t vgetFileSize(uint32_t fd);
