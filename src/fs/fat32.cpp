#include "fat32.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "../drivers/ahci.h"
#include <bit>

#define SECTOR_SIZE 512

BPB bpb_struct;
uint32_t first_data_sector = 0;

void init_fs() {
  read_from_disk((uint8_t*)&bpb_struct, 0, sizeof(BPB));
  uint32_t num_of_sectors = bpb_struct.large_sector_count;
  uint32_t fat_size = bpb_struct.num_of_sectors_per_fat;
  first_data_sector = bpb_struct.num_of_reserved_sectors + (bpb_struct.num_of_fats * bpb_struct.fat32_extention.sectors_per_fat);
  uint32_t first_fat_sector = bpb_struct.num_of_reserved_sectors;
  uint32_t num_of_data_sectors = num_of_sectors - (bpb_struct.num_of_reserved_sectors + fat_size * bpb_struct.num_of_fats);
}

uint32_t cluster_to_sector(uint32_t cluster) {
  return (cluster - 2) * bpb_struct.sectors_per_clusted + first_data_sector;
}

//@TODO: return the value of the entry instead of the offset to the entry
uint32_t getEntryByCluster(uint32_t cluster) {
  cluster *= 4;
  uint32_t fat_sector = bpb_struct.num_of_reserved_sectors + (cluster / SECTOR_SIZE);
  uint32_t entry_offset = cluster % SECTOR_SIZE;
  uint8_t sector_buff[512];
  read_from_disk(sector_buff, fat_sector*SECTOR_SIZE, 1*SECTOR_SIZE);
  uint32_t fat_entry = 0;
  memcpy((uint8_t*)&fat_entry, &sector_buff[entry_offset], sizeof(uint32_t));
  return fat_entry;
}

void writeEntryByCluster(uint32_t cluster, uint32_t value) {
  cluster *= 4;
  uint32_t fat_sector = bpb_struct.num_of_reserved_sectors + (cluster / SECTOR_SIZE);
  uint32_t entry_offset = cluster % SECTOR_SIZE;
  uint32_t entry_pos = fat_sector*bpb_struct.bytes_per_sector + entry_offset;
  write_to_disk((uint8_t*)&value, entry_pos, sizeof(value));
}

//@TODO: check if this actually needed
void iterateChain(uint32_t* chain, uint32_t cluster) {
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
}

FILE_DESCRIPTOR findFile(char* filename) {
  uint32_t curr_clust = 2;
  for(curr_clust = 2; curr_clust <= bpb_struct.large_sector_count; curr_clust++) {
    for(uint32_t sector = 0; sector < bpb_struct.sectors_per_clusted; sector++) {
      uint32_t curr_sector_num = cluster_to_sector(curr_clust);
      uint8_t curr_sector[bpb_struct.bytes_per_sector];
      read_from_disk(curr_sector, curr_sector_num*SECTOR_SIZE, 1*SECTOR_SIZE);

      for(uint32_t offset = 0; offset < bpb_struct.bytes_per_sector; offset+=32) {
        FILE_DESCRIPTOR* file_desc = std::bit_cast<FILE_DESCRIPTOR*>(curr_sector + offset);
        if(memcmp((uint8_t*)file_desc->filename, (uint8_t*)filename, 11) == 0) {
          return *file_desc; 
        }
      }
    }
  }

  return FILE_DESCRIPTOR{0};
}

void write_to_filedesc(char* filename, FILE_DESCRIPTOR fd) {
  uint32_t curr_clust = 2;
for(curr_clust = 2; curr_clust <= bpb_struct.large_sector_count; curr_clust++) {
    for(uint32_t sector = 0; sector < bpb_struct.sectors_per_clusted; sector++) {
      uint32_t curr_sector_num = cluster_to_sector(curr_clust);
      uint8_t curr_sector[bpb_struct.bytes_per_sector];
      read_from_disk(curr_sector, curr_sector_num*SECTOR_SIZE, 1*SECTOR_SIZE);

      for(uint32_t offset = 0; offset < bpb_struct.bytes_per_sector; offset+=32) {
        FILE_DESCRIPTOR* file_desc = (FILE_DESCRIPTOR*)(curr_sector + offset);
        if(memcmp((uint8_t*)file_desc->filename, (uint8_t*)filename, 11) == 0) {
          uint32_t position = (cluster_to_sector(curr_clust) + sector) * bpb_struct.bytes_per_sector + offset;
          write_to_disk((uint8_t*)&fd, position, sizeof(fd));
        }
      }
    }
  }
}

NAME_VALIDITY check_validity(const char* name) {
  const uint32_t name_len = strlen(name);
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
  
  if(name_len > 8) {
    for(uint8_t i = 0; i < name_len; i++) {
      if(name[i] == '.') {
        return NAME_WITH_EXT;
      }
    }

    return NAME_INVALID;
  }

  return NAME_WITHOUT_EXT;
}

void encode_name(const char* name, char* encoded_name) {
  if(name == NULL || encoded_name == NULL) {
    return;
  }

  uint32_t name_len = strlen(name);
  NAME_VALIDITY name_type = check_validity(name);
  if(name_type == NAME_INVALID) {
    return;
  }

  memset(encoded_name, ' ', 12);

  if(name_type == NAME_WITH_EXT) {
    uint8_t dot_idx = 0;
    for(uint32_t i = 0; i < strlen(name); i++) {
      if(name[i] == '.') {
        dot_idx = i;
      }
    }

    memcpy(encoded_name, name, dot_idx);
    memcpy(encoded_name + 8, name + dot_idx+1, 3);
  } else if(name_type == NAME_WITHOUT_EXT) {
    memcpy(encoded_name, name, name_len);
  }

  for(uint8_t i = 0; i < 12; i++) {
    encoded_name[i] = toupper(encoded_name[i]);
  }
}

uint8_t readFile(char* filename) {
  FILE_DESCRIPTOR file_desc = findFile(filename);
  FILE_DESCRIPTOR null_fd = {};
  if(memcmp(&file_desc, &null_fd, sizeof(file_desc)) == 0) {
    return 1;
  }

  uint8_t file_buff[file_desc.size_in_bytes];
  memset(file_buff, 0x0, file_desc.size_in_bytes);
  uint32_t fat_entry = file_desc.first_clust_low | (file_desc.first_clust_high >> 16);
  uint32_t offset = 0;
  while(fat_entry < 0x0FFFFFF8 && offset < file_desc.size_in_bytes) {
    uint32_t sector = cluster_to_sector(fat_entry);
    uint32_t size_to_read = file_desc.size_in_bytes - offset > 512 ? 512 : file_desc.size_in_bytes - offset;
    read_from_disk(file_buff + offset, sector*SECTOR_SIZE + offset, size_to_read);
    fat_entry = getEntryByCluster(fat_entry);
    offset += size_to_read;
  }

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

void createFile(char* filename) {
  uint32_t curr_clust = bpb_struct.fat32_extention.fat_cluster_num_of_root;
  FILE_DESCRIPTOR free_fd = {};
  uint32_t entry_pos = 0;
  memset((uint8_t*)&free_fd, 0x0, sizeof(FILE_DESCRIPTOR));

  // find free file entry in root directory
  while(curr_clust < 0x0FFFFFF8 && entry_pos == 0) {
    uint8_t clust_buff[bpb_struct.bytes_per_sector];
    memset(clust_buff, 0x0, bpb_struct.bytes_per_sector);
    read_from_disk(clust_buff, cluster_to_sector(curr_clust)*SECTOR_SIZE, bpb_struct.bytes_per_sector);

    for(uint32_t i = 0; i < SECTOR_SIZE; i+=32) {
      FILE_DESCRIPTOR* curr_fd = (FILE_DESCRIPTOR*)&clust_buff[i];
      
      if(memcmp((uint8_t*)&free_fd, (uint8_t*)curr_fd, sizeof(FILE_DESCRIPTOR)) == 0) {
        entry_pos = cluster_to_sector(curr_clust)*SECTOR_SIZE + i;
        break;
      }
    }

    curr_clust = getEntryByCluster(curr_clust);
  }

  // setup entry
  memcpy(free_fd.filename, filename, sizeof(free_fd.filename));

  uint32_t first_clust = allocate_cluster();
  free_fd.first_clust_low = first_clust & 0xFFFF;
  free_fd.first_clust_high = (first_clust >> 16) & 0xFFFF;
  
  // write entry to disk
  write_to_disk((uint8_t*)&free_fd, entry_pos, sizeof(free_fd));
}

uint8_t writeFile(char* filename, uint8_t* buff, uint32_t size) {
  FILE_DESCRIPTOR fd = findFile(filename);
  FILE_DESCRIPTOR null_fd = {};
  if(memcmp(&fd, &null_fd, sizeof(FILE_DESCRIPTOR)) == 0) {
    return 0;
  }
  
  uint32_t offset = 0;
  uint32_t first_file_cluster = fd.first_clust_low | (fd.first_clust_high >> 16);
  uint32_t last_file_cluster = get_last_cluster(first_file_cluster);
  uint32_t remaining_size = size;
  if(fd.size_in_bytes % 512 != 0 || fd.size_in_bytes == 0) {
    // write in remaning cluster space
    uint32_t remaining_clust_size = (bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted) - fd.size_in_bytes % 512;
    uint32_t size_to_write = size > remaining_clust_size ? remaining_clust_size : size;
    uint32_t position = cluster_to_sector(last_file_cluster)*bpb_struct.bytes_per_sector + fd.size_in_bytes % 512; 
    write_to_disk(buff, position, size_to_write);
    remaining_size -= size_to_write;
    offset += size_to_write;
  }

  // write rest of the file
  while(remaining_size > 0) {
    uint32_t new_clust = allocate_cluster();
    uint32_t size_to_write = remaining_size > bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted ? bpb_struct.bytes_per_sector*bpb_struct.sectors_per_clusted : remaining_size;
    write_to_disk(buff+offset, cluster_to_sector(new_clust), size_to_write);
    link_clusters(last_file_cluster, new_clust);
    last_file_cluster = new_clust;
    remaining_size -= size_to_write;
  }

  fd.size_in_bytes = fd.size_in_bytes + size;
  write_to_filedesc(filename, fd);
  return 0;
}

int main() {
  char buff1[50];
  read_from_disk((uint8_t*)buff1, 0x100400, 50);
  const char* name = "hello1.txt";
  char encoded_name[13] = {};
  encode_name(name, encoded_name);
  encoded_name[12] = '\00';
  std::cout << encoded_name << std::endl;
  init_fs();
  FSINFO buff{};
  //read_from_disk((uint8_t*)&buff, 1, sizeof(FSINFO));
  //FILE_DESCRIPTOR fd = findFile(encoded_name);
  //std::cout << fd.size_in_bytes << std::endl; 
  //createFile(encoded_name);
  memcpy(buff1, "hello world", 10);
  writeFile(encoded_name, (uint8_t*)buff1, 10);
  readFile(encoded_name);
}