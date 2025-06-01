#include "nvme.h"
#include "pci.h"
#include <cstdint>
#include "../pmm.h"
#include "../common.h"
#include "../mm/paging.h"
#include "../mm/npaging.h"
#include <bit>

#define NVME_NO_DATA_TRANSFER 0b00
#define NVME_HOST_TO_CONTROLLER_TRANSFER 0b01
#define NVME_CONTROLLER_TO_HOST_TRANSFER 0x10 
#define NVME_BI_DIRECTIONAL_TRANSFER 0b11

uint64_t nvme_base_addr = 0;
nvme_bar_s* nvme_bar{0};

void init_nvme() {
  pci_device_descriptor device_descriptor = {0};
  if(find_device(&device_descriptor, NVME_CLASSCODE, NVME_SUBCLASS, NVME_PROGIF) == false) {
    return; // no pci device found 
  }

  uint32_t bar0 = read_configuration_dword(device_descriptor.bus, device_descriptor.device, device_descriptor.function, 0x10);
  uint32_t bar1 = read_configuration_dword(device_descriptor.bus, device_descriptor.device, device_descriptor.function, 0x14);
  uint64_t nvme_base_addr = (uint64_t)(((uint64_t)bar1 << 32) | (bar0 & 0xFFFFFFF0));
  uint64_t nvme_virt_addr = 0;
  uint64_t phys_addr = 0;
  allocate_page(&nvme_virt_addr, &phys_addr, PRESENT_FLAG | RW_FLAG, 0);
  map_host_page(nvme_virt_addr, nvme_base_addr, PRESENT_FLAG | RW_FLAG); 
  nvme_bar = std::bit_cast<nvme_bar_s*>(nvme_virt_addr);

  pci_enable_dma(&device_descriptor);
  pci_enable_bus_mastering(&device_descriptor);

  // wait for the device to complete any resets
  while(getbit(&nvme_bar->controller_status, 0) == 1);
  create_admin_submision_queue();
  create_admin_completion_queue();
  // @TODO: set admin queue attributes
  
  // if CAP.CSS>NOIOCSS is set
  if(getbit((uint64_t*)&nvme_bar->cap, 51) == 1) {
    setbit(&nvme_bar->controller_config, 4);
    setbit(&nvme_bar->controller_config, 5);
    setbit(&nvme_bar->controller_config, 6);
  }

  // if CAP.CSS.IOCSS is cleared and CAP.CSS.NCSS is set
  if(getbit((uint64_t*)&nvme_bar->cap, 50) == 0 && getbit((uint64_t*)&nvme_bar->cap, 44) == 1) {
    clearbit((uint64_t*)&nvme_bar->cap, 4);
    clearbit((uint64_t*)&nvme_bar->cap, 5);
    clearbit((uint64_t*)&nvme_bar->cap, 6);

  // if CAP.CSS.IOCSS is set
  } else if(getbit((uint64_t*)&nvme_bar->cap, 50) == 1) {
    setbit(&nvme_bar->controller_config, 4);
    setbit(&nvme_bar->controller_config, 5);
    clearbit(&nvme_bar->controller_config, 6);
  }

  // set arbitation mechanism to round robin
  clearbit(&nvme_bar->controller_config, 11);
  clearbit(&nvme_bar->controller_config, 12);
  clearbit(&nvme_bar->controller_config, 13);

  // set memory page size to 0x1000
  clearbit(&nvme_bar->controller_config, 7);
  clearbit(&nvme_bar->controller_config, 8);
  clearbit(&nvme_bar->controller_config, 9);
  clearbit(&nvme_bar->controller_config, 10);

  // enable the controller 
  setbit(&nvme_bar->controller_config, 0);

  // wait for device to be ready
  while(getbit(&nvme_bar->controller_status, 0) == 0);

  uint64_t page = kpalloc();
  send_identify_command((uint8_t*)page, 1);
}

template<typename T>
T nvme_read_reg(uint32_t offset) {
  if(nvme_base_addr == 0) {
    return 0;
  }

  return *(T*)(nvme_base_addr + offset);
}

template<typename T>
void nvme_write_reg(uint32_t offset, T data) {
  if(nvme_base_addr == 0) {
    return;
  }

  *(T*)(nvme_base_addr + offset) = data;
}

uint32_t get_submission_doorbell_offset() {
  if(nvme_bar == 0x0) {
    return 0xFFFFFFFF;
  }

  return 0x1000 + 2 * (4 << nvme_bar->cap.dstrd);
}

uint32_t get_completion_doorbell_offset() {
  if(nvme_bar == 0x0) {
    return 0xFFFFFFFF;
  }

  return 0x1000 + 3 * (4 << nvme_bar->cap.dstrd);
}

uint32_t nvme_read_submission_doorbell() {
  if(nvme_base_addr == 0x0) {
    return 0xFFFFFFFF;
  }

  uint32_t offset = get_submission_doorbell_offset();
  return *(uint32_t*)(nvme_base_addr + offset);
}

uint32_t nvme_read_completion_doorbell() {
  if(nvme_base_addr == 0x0) {
    return 0xFFFFFFFF;
  }

  uint32_t offset = get_completion_doorbell_offset();
  return *(uint32_t*)(nvme_base_addr + offset);
}

void nvme_write_submission_doorbell(uint32_t data) {
  if(nvme_base_addr == 0x0) {
    return;
  }

  uint32_t offset = get_submission_doorbell_offset();
  *(uint32_t*)(nvme_base_addr + offset) = data;
}

void nvme_write_completion_doorbell(uint32_t data) {
  if(nvme_base_addr == 0x0) {
    return;
  }

  uint32_t offset = get_completion_doorbell_offset();
  *(uint32_t*)(nvme_base_addr + offset) = data;
}

void create_admin_submision_queue() {
  uint64_t submission_queue = kpalloc();
  nvme_bar->admin_subbmision_queue = submission_queue & 0xFFFFFFFFFFFFF000;
}

void create_admin_completion_queue() {
  uint64_t completion_queue = kpalloc();
  nvme_bar->admin_completion_queue = completion_queue & 0xFFFFFFFFFFFFF000;
}

void set_admin_queue_attrs(uint16_t completion_queue_size, uint16_t submission_queue_size) {
  nvme_bar->admin_queue_attr &= submission_queue_size;
  nvme_bar->admin_queue_attr &= (completion_queue_size << 16);
}

void send_identify_command(uint8_t* buff, uint8_t cns) {
  (void)buff;
  submision_queue_entry_s entry{0};
  submision_queue_entry_s comp_entry{0};
  entry.command.opcode = 0x6;
  entry.command.fused = 0;
  entry.command.prp_sgl = 0;
  entry.command.command_id = 0;
  entry.command.reserved = 0;
  
  entry.nsid = 0;
  entry.prp1 = (uint64_t)buff;
  entry.prp2 = 0;
  entry.metadata_ptr = 0;
  entry.reserved[0] = 0;
  entry.reserved[1] = 0;
  entry.command_specific[0] = cns;

  kmemcpy((uint8_t*)nvme_bar->admin_subbmision_queue, (uint8_t*)&entry, sizeof(entry));
  uint32_t doorbell_tail = nvme_read_submission_doorbell();
  doorbell_tail++;
  nvme_write_submission_doorbell(doorbell_tail);

  while(nvme_read_completion_doorbell() != 1);
  
  kmemcpy((uint8_t*)&comp_entry, (uint8_t*)nvme_bar->admin_completion_queue, sizeof(comp_entry));

}
