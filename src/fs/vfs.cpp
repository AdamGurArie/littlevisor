#include "vfs.h"
#include "fat32.h"
#include <cstdint>
#include <iterator>

#define MAX_NUMBER_OF_INODES 100

static VFILE_DESCRIPTOR file_descriptor_list[MAX_NUMBER_OF_INODES] = {};

static uint32_t findFreeInode() {
  for(uint32_t i = 0; i < MAX_NUMBER_OF_INODES; i++) {
    if(file_descriptor_list[i].occupied == 0) {
      file_descriptor_list[i].occupied = 1;
      return i;
    }
  }

  return OPEN_FILE_ERROR;
}

uint32_t vopenFile(char* filename) {
  if(!checkFileExists(filename)) {
    return OPEN_FILE_ERROR;
  }

  return findFreeInode();
}

uint32_t vreadFile(uint32_t fd, char* buff, uint32_t size) {
  if(fd >= 100) {
    return OPEN_FILE_ERROR;
  }

  VFILE_DESCRIPTOR fd_entry = file_descriptor_list[fd];
  return readFile(
      fd_entry.filename,
      (uint8_t*)buff,
      fd_entry.position,
      size
  );

}

uint32_t vwriteFile(uint32_t fd, char* buff, uint32_t size) {
  if(fd >= 100) {
    return OPEN_FILE_ERROR;
  }

  VFILE_DESCRIPTOR fd_entry = file_descriptor_list[fd];
  return writeFile(
      fd_entry.filename,
      (uint8_t*)buff,
      fd_entry.position,
      size
  );
}

uint32_t vseekp(uint32_t fd, uint32_t new_pos) {
  if(fd >= 100) {
    return OPEN_FILE_ERROR;
  }

  file_descriptor_list[fd].position = new_pos;
  return 0;
}

uint32_t vgetFileSize(uint32_t fd) {
  if(fd >= 100) {
    return OPEN_FILE_ERROR;
  }

  VFILE_DESCRIPTOR fd_entry = file_descriptor_list[fd];
  return getFileSize(fd_entry.filename);
}
