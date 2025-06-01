#include "fat32.h"
#include "../kheap.h"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <math.h>

// #include "../drivers/ahci.h"
#include "../drivers/ram_disk.h"
#include "../common.h"
#include <bit>
#include <sys/types.h>

#define SECTOR_SIZE 512

//@TODO: replace all variable-size arrays(those are illegal in cpp)

static BPB bpb_struct = {};
static uint32_t first_data_sector = 0;
static uint32_t number_of_clusters = 0;
static ahci* storage_dev = 0;

void encode_name(const char* name, char* encoded_name);
uint32_t traverse_cluster_chain(uint32_t clust, uint32_t num_of_clusters);

void init_fs(ahci* storage) {
  storage_dev = storage;
  storage_dev->read_data((uint8_t*)&bpb_struct, 0, sizeof(BPB));
  // uint32_t num_of_sectors = bpb_struct.large_sector_count;
  // uint32_t fat_size = bpb_struct.num_of_sectors_per_fat;
  first_data_sector = bpb_struct.num_of_reserved_sectors + (bpb_struct.num_of_fats * bpb_struct.fat32_extention.sectors_per_fat);
  number_of_clusters = (bpb_struct.large_sector_count - first_data_sector) / bpb_struct.sectors_per_clusted;
  // uint32_t first_fat_sector = bpb_struct.num_of_reserved_sectors;
  // uint32_t num_of_data_sectors = num_of_sectors - (bpb_struct.num_of_reserved_sectors + fat_size * bpb_struct.num_of_fats);
}

uint32_t cluster_to_sector(uint32_t cluster) {
  return (cluster - 2) * bpb_struct.sectors_per_clusted + first_data_sector;
}

//@TODO: return the value of the entry instead of the offset to the entry
uint32_t getEntryByCluster(uint32_t cluster) {
  cluster *= 4;
  uint32_t fat_sector = bpb_struct.num_of_reserved_sectors + (cluster / SECTOR_SIZE);
  uint32_t entry_offset = cluster % SECTOR_SIZE;
  uint8_t* sector_buff = (uint8_t*)kmalloc(storage_dev->get_sector_size());
  if(sector_buff == NULL) {
    return -1;
  }

  storage_dev->read_sector(fat_sector, sector_buff);
  uint32_t fat_entry = 0;
  kmemcpy((uint8_t*)&fat_entry, &sector_buff[entry_offset], sizeof(uint32_t));
  kfree(sector_buff);
  return fat_entry;
}

void writeEntryByCluster(uint32_t cluster, uint32_t value) {
  cluster *= 4;
  uint32_t fat_sector = bpb_struct.num_of_reserved_sectors + (cluster / SECTOR_SIZE);
  uint32_t entry_offset = cluster % SECTOR_SIZE;
  uint32_t entry_pos = fat_sector*bpb_struct.bytes_per_sector + entry_offset;
  storage_dev->write_data((uint8_t*)&value, entry_pos, sizeof(value));
}

//@TODO: check if this actually needed
/**void iterateChain(uint32_t* chain, uint32_t cluster) {
  while(1) {
    uint32_t addr_of_entry = getEntryByCluster(cluster);
    uint8_t sector_buff[SECTOR_SIZE];
    memset(sector_buff, 0x0, SECTOR_SIZE);
    read_from_disk(sector_buff, addr_of_entry, SECTOR_SIZE);
    memcpy((uint8_t*)&cluster, (uint8_t*)&sector_buff[addr_of_entry % SECTOR_SIZE], sizeof(uint32_t));
    if(cluster >= 0x0FFFFFF8) {
      break;
    }
  }
}**/

FILE_DESCRIPTOR findFile(const char* filename) {
  uint32_t curr_clust = 2;
  FILE_DESCRIPTOR fd;
  char encoded_filename[12];
  kmemset((uint8_t*)&fd, 0x0, sizeof(FILE_DESCRIPTOR));
  encode_name(filename, encoded_filename);

  for(curr_clust = 2; curr_clust <= number_of_clusters; curr_clust++) {
    for(uint32_t sector = 0; sector < bpb_struct.sectors_per_clusted; sector++) {
      uint32_t curr_sector_num = cluster_to_sector(curr_clust);
      uint8_t curr_sector[bpb_struct.bytes_per_sector];
      if(storage_dev->read_data(curr_sector, curr_sector_num*SECTOR_SIZE, 1*SECTOR_SIZE) == 1) {
        return fd;
      }

      for(uint32_t offset = 0; offset < bpb_struct.bytes_per_sector; offset+=32) {
        FILE_DESCRIPTOR* file_desc = std::bit_cast<FILE_DESCRIPTOR*>(curr_sector + offset);
        if(kmemcmp((uint8_t*)file_desc->filename, (uint8_t*)encoded_filename, 11) == 0) {
          return *file_desc; 
        }
      }
    }
  }

  return fd;
}

void write_to_filedesc(char* filename, FILE_DESCRIPTOR fd) {
  uint32_t curr_clust = 2;
  for(curr_clust = 2; curr_clust < number_of_clusters; curr_clust++) {
    for(uint32_t sector = 0; sector < bpb_struct.sectors_per_clusted; sector++) {
      uint32_t curr_sector_num = cluster_to_sector(curr_clust) + sector;
      uint8_t curr_sector[bpb_struct.bytes_per_sector];
      storage_dev->read_data(curr_sector, curr_sector_num*SECTOR_SIZE, 1*SECTOR_SIZE);

      for(uint32_t offset = 0; offset < bpb_struct.bytes_per_sector; offset+=32) {
        FILE_DESCRIPTOR* file_desc = std::bit_cast<FILE_DESCRIPTOR*>(curr_sector + offset);
        if(kmemcmp((uint8_t*)file_desc->filename, (uint8_t*)filename, 11) == 0) {
          uint32_t position = curr_sector_num * bpb_struct.bytes_per_sector + offset;
          storage_dev->write_data((uint8_t*)&fd, position, sizeof(fd));
          break;
        }
      }
    }
  }
}

NAME_VALIDITY check_validity(const char* name) {
  const uint32_t name_len = kstrlen(name);
  if(name_len > 12) {
    return NAME_INVALID;
  }

  for(uint8_t i = 0; i < name_len; i++) {
    if((name[i] >= 0x22 && name[i] < 0x2E) ||
       (name[i] >= 0x3A && name[i] <= 0x3F)) {

      return NAME_INVALID;
    } else if(name[i] == 0x5B || name[i] == 0x5C ||
              name[i] == 0x5D || name[i] == 0x7C) {

      return NAME_INVALID;
    }
  }
  
  
  for(uint8_t i = 0; i < name_len; i++) {
    if(name[i] == '.') {
      return NAME_WITH_EXT;
    }
  }

  return NAME_WITHOUT_EXT;
}

void encode_name(const char* name, char* encoded_name) {
  if(name == NULL || encoded_name == NULL) {
    return;
  }

  uint32_t name_len = kstrlen(name);
  NAME_VALIDITY name_type = check_validity(name);
  if(name_type == NAME_INVALID) {
    return;
  }

  kmemset((uint8_t*)encoded_name, ' ', 12);

  if(name_type == NAME_WITH_EXT) {
    uint8_t dot_idx = 0;
    for(uint32_t i = 0; i < kstrlen(name); i++) {
      if(name[i] == '.') {
        dot_idx = i;
      }
    }

    kmemcpy((uint8_t*)encoded_name, (uint8_t*)name, dot_idx);
    kmemcpy((uint8_t*)(encoded_name + 8), (uint8_t*)(name + dot_idx+1), 3);
  } else if(name_type == NAME_WITHOUT_EXT) {
    kmemcpy((uint8_t*)encoded_name, (uint8_t*)name, name_len);
  }

  for(uint8_t i = 0; i < 12; i++) {
    encoded_name[i] = ktoupper((char)encoded_name[i]);
  }
}

uint8_t readFile(char* filename, uint8_t* buff, uint32_t pos, uint32_t size) {
  FILE_DESCRIPTOR file_desc = findFile(filename);
  FILE_DESCRIPTOR null_fd;
  kmemset((uint8_t*)&null_fd, 0x0, sizeof(FILE_DESCRIPTOR));
  if(kmemcmp((uint8_t*)&file_desc, (uint8_t*)&null_fd, sizeof(file_desc)) == 0) {
    return 1;
  }

  uint8_t* file_buff = buff;
  if(size > file_desc.size_in_bytes) {
    size = file_desc.size_in_bytes;
  }

  kmemset(file_buff, 0x0, size);
  uint32_t fat_entry = file_desc.first_clust_low | (file_desc.first_clust_high >> 16);
  uint32_t offset = 0;
  uint32_t curr_sector = cluster_to_sector(fat_entry);
  uint32_t first_clust_of_data = pos / (bpb_struct.sectors_per_clusted * SECTOR_SIZE);
  uint32_t offset_of_data = pos % (bpb_struct.sectors_per_clusted * SECTOR_SIZE);

  for(uint32_t i = 0; i < first_clust_of_data; i++) {
    fat_entry = getEntryByCluster(fat_entry);
    if(fat_entry == 0x0FFFFFF8) {
      return 1;
    }
  }
  
  if(offset_of_data > 0) {
    uint32_t sector = cluster_to_sector(fat_entry);
    uint32_t size_to_read = SECTOR_SIZE - offset_of_data > size ? size : SECTOR_SIZE - offset_of_data;
    storage_dev->read_data(
        file_buff,
        sector * SECTOR_SIZE + offset_of_data,
        size_to_read
    );

    offset += SECTOR_SIZE - offset_of_data;
    fat_entry = getEntryByCluster(fat_entry);
  }

  while(fat_entry < 0x0FFFFFF8 && offset < size) {
    uint32_t sector = cluster_to_sector(fat_entry);
    for(uint32_t i = 0; i < bpb_struct.sectors_per_clusted; i++) {
      uint32_t size_to_read = size - offset > 512 ? 512 : size - offset;
      storage_dev->read_data(
          file_buff + offset,
          sector*SECTOR_SIZE,
          size_to_read
      );

      offset += size_to_read;
      if(offset >= size) {
        break;
      }
    }

    fat_entry = getEntryByCluster(fat_entry);
  }

  kmemcpy(buff, file_buff, size);

  return 0;
}

uint32_t allocate_cluster() {
  uint32_t free_cluster = 0;
  for(uint32_t cluster = 2; cluster < bpb_struct.large_sector_count; cluster++) {
    if(getEntryByCluster(cluster) == 0) {
      free_cluster = cluster;
      break;
    }
  }

  writeEntryByCluster(free_cluster, 0x0FFFFFF8);
  return free_cluster;
}

void link_clusters(uint32_t clust1, uint32_t clust2) {
  writeEntryByCluster(clust1, clust2);
}

uint32_t get_last_cluster(uint32_t clust) {
  uint32_t last_clust = clust;

  while(true) {
    clust = getEntryByCluster(last_clust);
    if(clust >= 0x0FFFFFF8) {
      break;
    }

    last_clust = clust;
  }

  return last_clust;
}

uint32_t traverse_cluster_chain(uint32_t clust, uint32_t num_of_clusters) {
  for(uint32_t i = 0; i < num_of_clusters; i++) {
    clust = getEntryByCluster(clust);
    if(clust >= 0x0FFFFFF8) {
      break;
    }

  }

  return clust;
}

void createFile(char* filename) {
  uint32_t curr_clust = bpb_struct.fat32_extention.fat_cluster_num_of_root;
  FILE_DESCRIPTOR free_fd;
  uint32_t entry_pos = 0;
  kmemset((uint8_t*)&free_fd, 0x0, sizeof(FILE_DESCRIPTOR));

  // find free file entry in root directory
  while(curr_clust < 0x0FFFFFF8 && entry_pos == 0) {
    uint8_t clust_buff[bpb_struct.bytes_per_sector];
    kmemset(clust_buff, 0x0, bpb_struct.bytes_per_sector);
    storage_dev->read_data(clust_buff, cluster_to_sector(curr_clust)*SECTOR_SIZE, bpb_struct.bytes_per_sector);

    for(uint32_t i = 0; i < SECTOR_SIZE; i+=32) {
      FILE_DESCRIPTOR* curr_fd = (FILE_DESCRIPTOR*)&clust_buff[i];
      
      if(kmemcmp((uint8_t*)&free_fd, (uint8_t*)curr_fd, sizeof(FILE_DESCRIPTOR)) == 0) {
        entry_pos = cluster_to_sector(curr_clust)*SECTOR_SIZE + i;
        break;
      }
    }

    curr_clust = getEntryByCluster(curr_clust);
  }

  // setup entry
  kmemcpy((uint8_t*)free_fd.filename, (uint8_t*)filename, sizeof(free_fd.filename));

  uint32_t first_clust = allocate_cluster();
  free_fd.first_clust_low = first_clust & 0xFFFF;
  free_fd.first_clust_high = (first_clust >> 16) & 0xFFFF;
  
  // write entry to disk
  storage_dev->read_data((uint8_t*)&free_fd, entry_pos, sizeof(free_fd));
}

// TODO: add support for pos argument
uint8_t writeFile(char* filename, uint8_t* buff, uint32_t pos, uint32_t size) {
  FILE_DESCRIPTOR fd = findFile(filename);
  FILE_DESCRIPTOR null_fd;
  kmemset((uint8_t*)&null_fd, 0x0, sizeof(FILE_DESCRIPTOR));
  if(kmemcmp((uint8_t*)&fd, (uint8_t*)&null_fd, sizeof(FILE_DESCRIPTOR)) == 0) {
    return 0;
  }
  
  uint32_t cluster_offset = pos / (bpb_struct.sectors_per_clusted * SECTOR_SIZE);
  uint32_t in_cluster_offset = pos % (bpb_struct.sectors_per_clusted * SECTOR_SIZE);

  uint32_t offset = 0;
  uint32_t first_file_cluster = fd.first_clust_low | (fd.first_clust_high >> 16);
  uint32_t write_cluster = traverse_cluster_chain(first_file_cluster, cluster_offset);
  uint32_t last_file_cluster = get_last_cluster(first_file_cluster);
  uint32_t remaining_size = size;

  if(fd.size_in_bytes % 512 != 0 || fd.size_in_bytes == 0) {
    // write in remaning cluster space
    uint32_t remaining_clust_size = (bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted) - fd.size_in_bytes % 512;
    uint32_t size_to_write = size > remaining_clust_size ? remaining_clust_size : size;
    uint32_t position = cluster_to_sector(write_cluster)*bpb_struct.bytes_per_sector + in_cluster_offset; 
    storage_dev->write_data(buff, position, size_to_write);
    remaining_size -= size_to_write;
    offset += size_to_write;
  }

  // write rest of the file
  while(remaining_size > 0) {
    write_cluster = traverse_cluster_chain(write_cluster, 1);
    if(write_cluster == last_file_cluster) {
      write_cluster = allocate_cluster();
      link_clusters(last_file_cluster, write_cluster);
      last_file_cluster = write_cluster;
    }

    uint32_t size_to_write = remaining_size > bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted ? bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted : remaining_size;
    storage_dev->write_data(buff+offset, cluster_to_sector(write_cluster), size_to_write);
    remaining_size -= size_to_write;
  }

  if(fd.size_in_bytes < (pos + size)) {
    fd.size_in_bytes = fd.size_in_bytes + size;
  }

  write_to_filedesc(fd.filename, fd);
  return 0;
}

uint32_t getFileSize(const char *filename) {
  FILE_DESCRIPTOR fd = findFile(filename);
  FILE_DESCRIPTOR null_fd;
  kmemset((uint8_t*)&null_fd, 0x0, sizeof(FILE_DESCRIPTOR));
  if(!kmemcmp((uint8_t*)&fd, (uint8_t*)&null_fd, sizeof(FILE_DESCRIPTOR))) {
    return 0;
  }

  return fd.size_in_bytes;
}

// @TODO
bool checkFileExists(const char* filename) {
  FILE_DESCRIPTOR fd = findFile(filename);
  FILE_DESCRIPTOR null_fd;
  kmemset((uint8_t*)&null_fd, 0x0, sizeof(FILE_DESCRIPTOR));

  if(!kmemcmp((uint8_t*)&fd, (uint8_t*)&null_fd, sizeof(FILE_DESCRIPTOR))) {
    return false;
  }

  return true;
}
