#pragma once

#include <cstdint>
#include "../drivers/storage_device.h"
#include "../drivers/ram_disk.h"

struct FAT32 {
  uint32_t sectors_per_fat;
  uint16_t flags;
  uint16_t fat_version;
  uint32_t fat_cluster_num_of_root;
  uint16_t sector_num_of_fsinfo;
  uint16_t sector_num_of_backup_boot_sector;
  uint32_t reserved[3];
  uint8_t drive_num;
  uint8_t reserved_1;
  uint8_t signature;
  uint32_t volume_id;
  uint8_t volume_label_string[11];
  uint8_t system_identifier_string[8];
} __attribute__((packed));

struct BPB {
  uint8_t jpm_inst[3];
  uint8_t oem_identifier[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_clusted;
  uint16_t num_of_reserved_sectors;
  uint8_t num_of_fats;
  uint16_t num_of_root_dir_entries;
  uint16_t num_of_sectors_in_logical_volume;
  uint8_t media_descriptor_type;
  uint16_t num_of_sectors_per_fat;
  uint16_t num_of_sectors_per_track;
  uint16_t num_of_heads;
  uint32_t num_of_hidden_sectors;
  uint32_t large_sector_count;
  FAT32 fat32_extention;
} __attribute__((packed));

struct FSINFO {
  uint8_t signature[4];
  uint8_t reserved_1[480];
  uint8_t signature_2[4];
  uint32_t last_known_free_clust;
  uint32_t start_cluster;
  uint32_t reserved[3];
  uint8_t trail_signature[4];
} __attribute__((packed));

struct DIR_ENTRY {
  char short_name[11];
  uint8_t dir_attr;
  uint8_t dir_ntres;
  uint8_t dir_crttimetenth;
  uint16_t dir_crttime;
  uint16_t dir_crtdate;
  uint16_t dir_lstaccdate;
  uint16_t dir_fst_clus_hi;
  uint16_t dir_wrttime;
  uint16_t dir_wrtdate;
  uint16_t fstclus_lo;
  uint8_t dir_filesize;
} __attribute__((packed));

struct FILE_DESCRIPTOR {
  char filename[11];
  uint8_t attributes;
  uint8_t reserved_for_nt;
  uint8_t creation_time_hund_secs;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t last_accessed_date;
  uint16_t first_clust_high;
  uint16_t last_modification_time;
  uint16_t last_modification_date;
  uint16_t first_clust_low;
  uint32_t size_in_bytes;
} __attribute__((packed));

enum NAME_VALIDITY {
  NAME_INVALID,
  NAME_WITH_EXT,
  NAME_WITHOUT_EXT,
};

void init_fs(ramDisk* storage_dev);
uint8_t writeFile(char* filename, uint8_t* buff, uint32_t pos, uint32_t size);
uint8_t readFile(char* filename, uint8_t* buff, uint32_t pos, uint32_t size);
uint32_t getFileSize(const char* filename);
bool checkFileExists(const char* filename);
