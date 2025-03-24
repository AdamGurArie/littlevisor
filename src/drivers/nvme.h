#include <cstdint>
#define NVME_CLASSCODE 0x1
#define NVME_SUBCLASS 0x8
#define NVME_PROGIF 0x2

#include "pci.h"

struct cap_register_s {
  uint16_t mqes;
  uint8_t cqr : 1;
  uint8_t ams : 2;
  uint8_t reserved : 5;
  uint8_t timeout;
  uint8_t dstrd : 4;
  uint8_t nssrs : 1;
  uint8_t css;
  uint8_t bps : 1;
  uint8_t cps : 2;
  uint8_t mpsmin : 4;
  uint8_t mspmax : 4;
  uint8_t pmrs : 1;
  uint8_t cmbs : 1;
  uint8_t nsss : 1;
  uint8_t crms : 2;
  uint8_t nsses : 1;
  uint8_t reserved1 : 2;
} __attribute__((packed));

struct nvme_bar_s {
  cap_register_s cap;
  uint32_t version;
  uint32_t intms;
  uint32_t intmc;
  uint32_t controller_config;
  uint32_t controller_status;
  uint32_t admin_queue_attr;
  uint64_t admin_subbmision_queue;
  uint64_t admin_completion_queue;
} __attribute__((packed));

struct opcode_s {
  uint8_t data_direction : 2;
  uint8_t function : 5;
} __attribute__((packed));

struct command_s {
  uint8_t opcode;
  uint8_t fused : 2;
  uint8_t reserved : 4;
  uint8_t prp_sgl : 2;
  uint16_t command_id;
} __attribute__((packed));

struct submision_queue_entry_s {
  command_s command;
  uint32_t nsid;
  uint32_t reserved[2];
  uint64_t metadata_ptr;
  uint64_t prp1;
  uint64_t prp2;
  uint32_t command_specific[6];
} __attribute__((packed));

template<typename T>
T nvme_read_reg(uint32_t offset);

template<typename T>
void nvme_write_reg(uint32_t offset, T data);

void init_nvme();
void create_admin_submision_queue();
void create_admin_completion_queue();
void send_identify_command(uint8_t* buff, uint8_t cns);
