#include "ahci.h"
#include "pci.h"
#include "../mm/npaging.h"
#include "../common.h"
#include "../pmm.h"

#include <bit>
#include <cstdint>
#include <math.h>
#include <sys/types.h>

#define AHCI_CLASS 0x1
#define AHCI_SUBCLASS 0x6
#define PROG_IF 0x1

#define HBA_PxCMD_ST_OFF    0x0
#define HBA_PxCMD_SUD_OFF   0x1
#define HBA_PxCMD_FRE_OFF   0x4
#define HBA_PxCMD_FR_OFF    0x7
#define HBA_PxCMD_CR_OFF    0x8

#define HBA_PxSCTL_DET_OFF  0x0

#define HBA_PxTFD_STS_ERR_OFF 0x0
#define HBA_PxTFD_STS_DRQ_OFF 0x3
#define HBA_PxTFD_STS_BSY_OFF 0x7

#define HBA_PxSSTS_DET_OFF 0x0

#define SECTOR_SIZE 512
#define MAX_PRDT_SIZE 0x4000

#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35
#define ATA_CMD_IDENTIFY        0xec

// static hba_mem_regs* ahci_hba = 0;
// static port_type port_types[32] = {};
// static uint32_t sata_device_port = 0;

void init_port(uint32_t port);
void map_ports();

void stop_command_engine(uint32_t port);
void start_command_engine(uint32_t port);

void commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write);

uint64_t ahci::get_sector_size() {
  return SECTOR_SIZE;
}

ahci::ahci() {
  init_ahci();
}

void ahci::init_ahci() {
  common_pci_header* ahci_common_header = find_device(AHCI_CLASS, AHCI_SUBCLASS, PROG_IF);
  if(ahci_common_header->header_type != 0x0) {
    //panic
    return;
  }

  setbit((uint64_t*)&ahci_common_header->cmd_reg, 1);
  clearbit((uint64_t*)&ahci_common_header->cmd_reg, 10);

  header_type_0* ahci_header = std::bit_cast<header_type_0*>(ahci_common_header);
  ahci_hba = std::bit_cast<hba_mem_regs*>(TO_HIGHER_HALF(ahci_header->bar5));
  // init all ports
  for(uint32_t i = 0; i < 32; i++) {
    if(getbit(ahci_hba->generic_host_control.pi, i) == 1) {
      init_port(i);
    }
  }
  // map all their types
  map_ports();
  // pick the first one that is regular sata(support for using many devices in parallel will come in the future)
  for(uint32_t i = 0; i < 32; i++) {
    if(port_types[i] == SATA_SIG_ATA) {
      sata_device_port = i;
    }
  }
}

uint8_t ahci::read_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  return 0;
}

uint8_t ahci::write_data(uint8_t* buff, uint64_t offset, uint32_t size) {
  return 0;
}

void ahci::init_port(uint32_t port) {
  if(port > 31) {
    return;
  }

  port_register_struct* port_reg = &ahci_hba->port_registers[port];
  // stop command engine
  stop_command_engine(port);
  // allocate phys memory for command list
  uint64_t cmd_list_addr = kpalloc();
  kmemset((uint8_t*)TO_HIGHER_HALF(cmd_list_addr), 0x0, 0x1000);
  port_reg->pxclb = cmd_list_addr & 0xFFFFFFFF;
  port_reg->pxclbu = (cmd_list_addr >> 32) & 0xFFFFFFFF;
  
  command_header* cmd_hdr = std::bit_cast<command_header*>(TO_HIGHER_HALF(cmd_list_addr));
  for(uint32_t i = 0; i < 32; i++) {
    cmd_hdr[i].prdtl = 8;
    uint64_t cmd_table_addr = kpalloc();
    kmemset((uint8_t*)TO_HIGHER_HALF(cmd_list_addr), 0x0, 0x1000);
    cmd_hdr[i].ctba = cmd_table_addr & 0xFFFFFFFF;
    cmd_hdr[i].ctbau = (cmd_table_addr >> 32) & 0xFFFFFFFF;
  }
  // allocate phys memory for FIS area
  uint64_t fis_area_addr = kpalloc();
  kmemset((uint8_t*)TO_HIGHER_HALF(fis_area_addr), 0x0, 0x1000);
  port_reg->pxfb = fis_area_addr & 0xFFFFFFFF;
  port_reg->pxfbu = (fis_area_addr >> 32) & 0xFFFFFFFF; 
  // set pxcmd.fre
  uint64_t reg = port_reg->pxcmd;
  setbit((uint64_t*)&reg, HBA_PxCMD_FRE_OFF);
  port_reg->pxcmd = reg;
  // initiate a spin up(set pxcmd.sud 1)

  /**reg = port_reg->pxcmd;
  setbit((uint64_t*)&reg, HBA_PxCMD_SUD_OFF);
  port_reg->pxcmd = reg;

  // wait for an indications that device is connected(check how to do that)
  reg = port_reg->pxssts;
  while(getbit((uint64_t*)&reg, HBA_PxSSTS_DET_OFF) != 0x1 && getbit((uint64_t*)&reg, HBA_PxSCTL_DET_OFF) != 0x3) {
    reg = port_reg->pxssts;
  }
  // clear pxserr
  port_reg->pxserr = 0xFFFFFFFF;
  // wait for indication that sata is ready(check how to do it)
  reg = port_reg->pxtfd;
  while(getbit((uint64_t*)&reg, HBA_PxTFD_STS_ERR_OFF) == 0
     && getbit((uint64_t*)&reg, HBA_PxTFD_STS_DRQ_OFF) == 0 
     && getbit((uint64_t*)&reg, HBA_PxTFD_STS_BSY_OFF) == 0) {
    reg = port_reg->pxtfd;
  }**/

  start_command_engine(port);
}

void ahci::stop_command_engine(uint32_t port) {
  if(port > 31) {
    return;
  }

  port_register_struct* port_reg = &ahci_hba->port_registers[port];
  uint64_t reg = port_reg->pxcmd;
  if(getbit((uint64_t*)&reg, HBA_PxCMD_ST_OFF) == 0) {
    return;
  }
  // check if st != 1
  // yes? exit
  
  clearbit((uint64_t*)&reg, HBA_PxCMD_ST_OFF);
  clearbit((uint64_t*)&reg, HBA_PxCMD_FRE_OFF);
  port_reg->pxcmd = 0xc006; //(uint32_t)(reg & 0xFFFFFFFF);

  while(getbit((uint64_t*)&reg, HBA_PxCMD_FR_OFF) == 0 && getbit((uint64_t)&reg, HBA_PxCMD_CR_OFF) == 0) {
    reg = port_reg->pxcmd;
  }
  
  reg = port_reg->pxsctl;
  clearbit((uint64_t*)&reg, HBA_PxSCTL_DET_OFF);
  port_reg->pxsctl = reg;
  // no: continue
  // clear pxcmd.st
  // clear pxcmd.fre
  // wait until pxcmd.fr and pxcmd.cr are clear
  // clear pxsctl.det
}

void ahci::map_ports() {
  uint32_t ports_available = ahci_hba->generic_host_control.pi;
  for(uint32_t i = 0; i < 32; i++) {
    port_register_struct* port_reg = &ahci_hba->port_registers[i];
    if(getbit((uint64_t)ports_available, i) == 1) {
      if(port_reg->pxsig == SATA_SIG_ATA) {
        port_types[i] = SATA_SIG_ATA;
} else if(port_reg->pxsig == SATA_SIG_ATAPI) {
        port_types[i] = SATA_SIG_ATAPI;
      } else if(port_reg->pxsig == SATA_SIG_PM) {
        port_types[i] = SATA_SIG_PM;
      } else if(port_reg->pxsig == SATA_SIG_SEMB) {
        port_types[i] = SATA_SIG_SEMB;
      }
    } else {
      port_types[i] = PORT_NOT_AVAILABLE; 
    }
  }
}

void ahci::start_command_engine(uint32_t port) {
  if(port > 31) {
    return;
  }

  port_register_struct* port_reg = &ahci_hba->port_registers[port];
  uint64_t reg = port_reg->pxcmd;
  while(getbit((uint64_t*)&reg, HBA_PxCMD_CR_OFF) == 1) {
    reg = port_reg->pxcmd;
  }

  setbit((uint64_t*)&reg, HBA_PxCMD_FRE_OFF);
  setbit((uint64_t*)&reg, HBA_PxCMD_ST_OFF);

  port_reg->pxcmd = reg;
}

void ahci::commit_transaction(uint8_t* buff, uint64_t start_sector, uint16_t num_of_sectors, bool write) {
  port_register_struct* port_reg = &ahci_hba->port_registers[sata_device_port];
  // find available command slot
  uint64_t pxsact = port_reg->pxsact;
  uint64_t pxci = port_reg->pxci;
  uint32_t cmd_slot = 0;
  for(cmd_slot = 0; cmd_slot < 32; cmd_slot++) {
    if(getbit((uint64_t)pxsact, cmd_slot) == 0 && getbit((uint64_t)pxci, cmd_slot) == 0) {
      break; 
    }
  }

  // build command FIS
  command_header* cmd_hdr = (command_header*)(TO_HIGHER_HALF((port_reg->pxclb | ((uint64_t)port_reg->pxclbu << 32))));
  cmd_hdr->cfl = sizeof(FIS_REG_H2D);
  cmd_hdr->atapi = 0;
  cmd_hdr->write = write ? 1 : 0;
  cmd_hdr->prefetchable = 0;
  cmd_hdr->pmp = 0;
  cmd_hdr->prdtl = (num_of_sectors*SECTOR_SIZE) / 512;
  
  command_table* cmd_tbl = (command_table*)(TO_HIGHER_HALF((cmd_hdr->ctba | ((uint64_t)cmd_hdr->ctbau << 32))));

  uint16_t i = 0;
  for(i = 0; i < cmd_hdr->prdtl - 1; i++) {
    cmd_tbl->prdt_list[i].dba = (uint64_t)buff & 0xFFFFFFFF;
    cmd_tbl->prdt_list[i].dbau = ((uint64_t)buff >> 32) & 0xFFFFFFFF;
    cmd_tbl->prdt_list[i].dbc = 8*1024 - 1;
    cmd_tbl->prdt_list[i].ioc = 0;
    buff += 4*1024;
    num_of_sectors -= 16;
  }

  cmd_tbl->prdt_list[i].dba = (uint64_t)buff & 0xFFFFFFFF;
  //cmd_tbl->prdt_list[i].dbau = ((uint64_t)buff >> 32) & 0xFFFFFFFF;
  cmd_tbl->prdt_list[i].dbc = num_of_sectors * 512 - 1;
  cmd_tbl->prdt_list[i].ioc = 0;

  FIS_REG_H2D* fis = (FIS_REG_H2D*)&cmd_tbl->cfis;
  kmemset((uint8_t*)fis, 0x0, sizeof(FIS_REG_H2D));
  fis->fis_type = REG_H2D;
  fis->command = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;
  fis->countl = num_of_sectors & 0xFF;
  fis->counth = (num_of_sectors >> 8) & 0xFF;
  fis->c = 1;
  fis->icc = 0;
  fis->pmport = 0;
  fis->lba0 = start_sector & 0xFF;
  fis->lba1 = (start_sector >> 8) & 0xFF;
  fis->lba1 = (start_sector >> 16) & 0xFF;
  fis->lba2 = (start_sector >> 24) & 0xFF;
  fis->lba3 = (start_sector >> 32) & 0xFF;
  fis->lba4 = (start_sector >> 40) & 0xFF;
  fis->lba5 = (start_sector >> 48) & 0xFF;
  fis->device = 1<<6;
  port_reg->pxie = 0xFFFFFFFF;

  port_reg->pxci |= 1<<cmd_slot; 
  while((port_reg->pxci & (1<<cmd_slot)) == 1);
}

void ahci::read_from_disk(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  uint16_t real_size = size / 512;
  uint8_t wrap_buff[real_size*512];
  kmemset(wrap_buff, 0x0, sizeof(wrap_buff));
  commit_transaction(wrap_buff, start_sector, real_size, false);
  kmemcpy(buff, wrap_buff, size);
}

void ahci::write_to_disk(uint8_t* buff, uint64_t start_sector, uint16_t size) {
  uint16_t real_size = size / 512;
  //kmemcpy(wrap_buff, buff, size);
  commit_transaction(buff, start_sector, real_size, true);
  //kmemcpy(buff, wrap_buff, size);
}

