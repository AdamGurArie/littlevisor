#pragma once

#include "fat32.h"

uint32_t openFile(char* filename);
uint32_t readFile(uint32_t fd, char* buff, uint32_t size);
uint32_t writeFile(uint32_t fd, char* buff, uint32_t size);
uint32_t seekp(uint32_t fd, uint32_t new_pos);
uint32_t getFileSize(uint32_t fd);
