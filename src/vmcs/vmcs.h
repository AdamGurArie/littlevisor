#pragma once

//#include "../vm_drivers/ide.h"
#include "../vm_drivers/ata_pio.h"
#include "../vm_drivers/cmos.h"
#include <cstdint>
#include <sys/cdefs.h>

#define VMCB_STATESAVE_AREA_OFFSET 0x3FF
#define MEMORY_SPACE_PER_VM 20000000

#define MEM_SIZE_PER_VM 0x4000000
#define MEM_SIZE_COREBOOT 0x1000000

enum vmcb_registers {
  RAX,
};

enum event_type_e {
  INTR = 0,
  NMI = 2,
  EXCEPTION = 3,
  SINTR = 4
};

struct segment_selector {
  uint16_t selector;
  uint16_t attrib;
  uint32_t limit;
  uint64_t base;
} __attribute__((packed));

struct event_injection_field {
  uint8_t vector;
  uint8_t type : 3;
  uint8_t error_code_valid : 1;
  uint32_t reserved : 19;
  uint8_t valid : 1;
  uint32_t errorcode;
} __attribute__((packed));

struct guest_regs {
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t dr0;
  uint64_t dr1;
  uint64_t dr2;
  uint64_t dr3;
} __attribute__((packed));

struct vmcb_control {
  uint16_t intercept_cr_reads;
  uint16_t intercepts_cr_writes;
  uint16_t intercepts_dr_reads;
  uint16_t intercepts_dr_writes;
  uint32_t intercept_exceptions;
  uint32_t intercepts_insts_1;
  uint32_t intercepts_insts_2;
  uint8_t intercepts_insts_3;
  uint8_t reserved_1[39];
  uint16_t pause_filter_threshold;
  uint16_t pause_filter_count;
  uint64_t iopm_base_pa;
  uint64_t msrpm_base_pa;
  uint64_t tsc_offset;
  uint32_t guest_asid;
  uint32_t tlb_control;
  uint64_t vintr;
  uint8_t interrupt_shadow : 1;
  uint8_t guest_interrupt_mask : 1;
  uint64_t reserved_7 : 62;
  uint64_t exitcode;
  uint64_t exitinfo1;
  uint64_t exitinfo2;
  uint64_t exitintinfo;
  uint64_t np_enable : 1;
  uint64_t reserved_12 : 63;
  uint64_t avic_apic_bar_reserved;
  uint64_t guest_phys_addr_ghcb;
  event_injection_field eventinj;
  uint64_t n_cr3;
  uint64_t lbr_virtualization_enable;
  uint32_t vmcb_clean_bits;
  uint32_t reserved_11;
  uint64_t nrip;
  uint8_t number_of_bytes_fetched;
  uint8_t guest_instruction_bytes[15];
  uint64_t avic_apic_backing_page_ptr;
  uint64_t reserved_13;
  uint64_t avic_logical_table_ptr;
  uint64_t avic_physical_table_ptr;
  uint64_t reserved_14;
  uint64_t vmcb_save_state_ptr;
  uint8_t reserved26[752];
} __attribute__((packed));

struct vmcb_state_save_area {
  struct segment_selector es;
  struct segment_selector cs;
  struct segment_selector ss;
  struct segment_selector ds;
  struct segment_selector fs;
  struct segment_selector gs;
  struct segment_selector gdtr;
  struct segment_selector ldtr;
  struct segment_selector idtr;
  struct segment_selector tr;
  uint8_t reserved1[0x0cb - 0x0a0];
  uint8_t cpl;                          // +0x0cb
  uint32_t reserved2;                   // +0x0cc
  uint64_t efer;                        // +0x0d0
  uint8_t reserved3[0x148 - 0x0d8];     // +0x0d8
  uint64_t cr4;                         // +0x148
  uint64_t cr3;                         // +0x150
  uint64_t cr0;                         // +0x158
  uint64_t dr7;                         // +0x160
  uint64_t dr6;                         // +0x168
  uint64_t rflags;                      // +0x170
  uint64_t rip;                         // +0x178
  uint8_t reserved4[0x1d8 - 0x180];     // +0x180
  uint64_t rsp;                         // +0x1d8
  uint8_t reserved5[0x1f8 - 0x1e0];     // +0x1e0
  uint64_t rax;                         // +0x1f8
  uint64_t star;                        // +0x200
  uint64_t lstar;                       // +0x208
  uint64_t cstar;                       // +0x210
  uint64_t sfMask;                      // +0x218
  uint64_t kernelgsbase;                // +0x220
  uint64_t sysentercs;                  // +0x228
  uint64_t sysenteresp;                 // +0x230
  uint64_t sysentereip;                 // +0x238
  uint64_t cr2;                         // +0x240
  uint8_t reserved6[0x268 - 0x248];     // +0x248
  uint64_t gpat;                        // +0x268
  uint64_t dbgctl;                      // +0x270
  uint64_t brfrom;                      // +0x278
  uint64_t brto;                        // +0x280
  uint64_t lastexcepfrom;               // +0x288
  uint64_t lastexcepto;                 // +0x290
} __attribute__((packed));

struct vmcb {
  vmcb_control control;
  vmcb_state_save_area state_save_area;
} __attribute__((packed));

struct context {
  vmcb_state_save_area guest;
  guest_regs regs;
  uint64_t guest_cr3;
  uint32_t guest_asid;
  ata_pio_device ata_device;
  cmos_device* cmos_dev;
};

enum VMEXIT_EXITCODE
{
    /* control register read exitcodes */
    VMEXIT_CR0_READ    =   0,
    VMEXIT_CR1_READ    =   1,
    VMEXIT_CR2_READ    =   2,
    VMEXIT_CR3_READ    =   3,
    VMEXIT_CR4_READ    =   4,
    VMEXIT_CR5_READ    =   5,
    VMEXIT_CR6_READ    =   6,
    VMEXIT_CR7_READ    =   7,
    VMEXIT_CR8_READ    =   8,
    VMEXIT_CR9_READ    =   9,
    VMEXIT_CR10_READ   =  10,
    VMEXIT_CR11_READ   =  11,
    VMEXIT_CR12_READ   =  12,
    VMEXIT_CR13_READ   =  13,
    VMEXIT_CR14_READ   =  14,
    VMEXIT_CR15_READ   =  15,

    /* control register write exitcodes */
    VMEXIT_CR0_WRITE   =  16,
    VMEXIT_CR1_WRITE   =  17,
    VMEXIT_CR2_WRITE   =  18,
    VMEXIT_CR3_WRITE   =  19,
    VMEXIT_CR4_WRITE   =  20,
    VMEXIT_CR5_WRITE   =  21,
    VMEXIT_CR6_WRITE   =  22,
    VMEXIT_CR7_WRITE   =  23,
    VMEXIT_CR8_WRITE   =  24,
    VMEXIT_CR9_WRITE   =  25,
    VMEXIT_CR10_WRITE  =  26,
    VMEXIT_CR11_WRITE  =  27,
    VMEXIT_CR12_WRITE  =  28,
    VMEXIT_CR13_WRITE  =  29,
    VMEXIT_CR14_WRITE  =  30,
    VMEXIT_CR15_WRITE  =  31,

    /* debug register read exitcodes */
    VMEXIT_DR0_READ    =  32,
    VMEXIT_DR1_READ    =  33,
    VMEXIT_DR2_READ    =  34,
    VMEXIT_DR3_READ    =  35,
    VMEXIT_DR4_READ    =  36,
    VMEXIT_DR5_READ    =  37,
    VMEXIT_DR6_READ    =  38,
    VMEXIT_DR7_READ    =  39,
    VMEXIT_DR8_READ    =  40,
    VMEXIT_DR9_READ    =  41,
    VMEXIT_DR10_READ   =  42,
    VMEXIT_DR11_READ   =  43,
    VMEXIT_DR12_READ   =  44,
    VMEXIT_DR13_READ   =  45,
    VMEXIT_DR14_READ   =  46,
    VMEXIT_DR15_READ   =  47,

    /* debug register write exitcodes */
    VMEXIT_DR0_WRITE   =  48,
    VMEXIT_DR1_WRITE   =  49,
    VMEXIT_DR2_WRITE   =  50,
    VMEXIT_DR3_WRITE   =  51,
    VMEXIT_DR4_WRITE   =  52,
    VMEXIT_DR5_WRITE   =  53,
    VMEXIT_DR6_WRITE   =  54,
    VMEXIT_DR7_WRITE   =  55,
    VMEXIT_DR8_WRITE   =  56,
    VMEXIT_DR9_WRITE   =  57,
    VMEXIT_DR10_WRITE  =  58,
    VMEXIT_DR11_WRITE  =  59,
    VMEXIT_DR12_WRITE  =  60,
    VMEXIT_DR13_WRITE  =  61,
    VMEXIT_DR14_WRITE  =  62,
    VMEXIT_DR15_WRITE  =  63,

    /* processor exception exitcodes (VMEXIT_EXCP[0-31]) */
    VMEXIT_EXCEPTION_DE  =  64, /* divide-by-zero-error */
    VMEXIT_EXCEPTION_DB  =  65, /* debug */
    VMEXIT_EXCEPTION_NMI =  66, /* non-maskable-interrupt */
    VMEXIT_EXCEPTION_BP  =  67, /* breakpoint */
    VMEXIT_EXCEPTION_OF  =  68, /* overflow */
    VMEXIT_EXCEPTION_BR  =  69, /* bound-range */
    VMEXIT_EXCEPTION_UD  =  70, /* invalid-opcode*/
    VMEXIT_EXCEPTION_NM  =  71, /* device-not-available */
    VMEXIT_EXCEPTION_DF  =  72, /* double-fault */
    VMEXIT_EXCEPTION_09  =  73, /* unsupported (reserved) */
    VMEXIT_EXCEPTION_TS  =  74, /* invalid-tss */
    VMEXIT_EXCEPTION_NP  =  75, /* segment-not-present */
    VMEXIT_EXCEPTION_SS  =  76, /* stack */
    VMEXIT_EXCEPTION_GP  =  77, /* general-protection */
    VMEXIT_EXCEPTION_PF  =  78, /* page-fault */
    VMEXIT_EXCEPTION_15  =  79, /* reserved */
    VMEXIT_EXCEPTION_MF  =  80, /* x87 floating-point exception-pending */
    VMEXIT_EXCEPTION_AC  =  81, /* alignment-check */
    VMEXIT_EXCEPTION_MC  =  82, /* machine-check */
    VMEXIT_EXCEPTION_XF  =  83, /* simd floating-point */

    /* exceptions 20-31 (exitcodes 84-95) are reserved */

    /* ...and the rest of the #VMEXITs */
    VMEXIT_INTR             =  96,
    VMEXIT_NMI              =  97,
    VMEXIT_SMI              =  98,
    VMEXIT_INIT             =  99,
    VMEXIT_VINTR            = 100,
    VMEXIT_CR0_SEL_WRITE    = 101,
    VMEXIT_IDTR_READ        = 102,
    VMEXIT_GDTR_READ        = 103,
    VMEXIT_LDTR_READ        = 104,
    VMEXIT_TR_READ          = 105,
    VMEXIT_IDTR_WRITE       = 106,
    VMEXIT_GDTR_WRITE       = 107,
    VMEXIT_LDTR_WRITE       = 108,
    VMEXIT_TR_WRITE         = 109,
    VMEXIT_RDTSC            = 110,
    VMEXIT_RDPMC            = 111,
    VMEXIT_PUSHF            = 112,
    VMEXIT_POPF             = 113,
    VMEXIT_CPUID            = 114,
    VMEXIT_RSM              = 115,
    VMEXIT_IRET             = 116,
    VMEXIT_SWINT            = 117,
    VMEXIT_INVD             = 118,
    VMEXIT_PAUSE            = 119,
    VMEXIT_HLT              = 120,
    VMEXIT_INVLPG           = 121,
    VMEXIT_INVLPGA          = 122,
    VMEXIT_IOIO             = 123,
    VMEXIT_MSR              = 124,
    VMEXIT_TASK_SWITCH      = 125,
    VMEXIT_FERR_FREEZE      = 126,
    VMEXIT_SHUTDOWN         = 127,
    VMEXIT_VMRUN            = 128,
    VMEXIT_VMMCALL          = 129,
    VMEXIT_VMLOAD           = 130,
    VMEXIT_VMSAVE           = 131,
    VMEXIT_STGI             = 132,
    VMEXIT_CLGI             = 133,
    VMEXIT_SKINIT           = 134,
    VMEXIT_RDTSCP           = 135,
    VMEXIT_ICEBP            = 136,
    VMEXIT_WBINVD           = 137,
    VMEXIT_MONITOR          = 138,
    VMEXIT_MWAIT            = 139,
    VMEXIT_MWAIT_CONDITIONAL= 140,
    VMEXIT_NPF              = 1024, /* nested paging fault */
    VMEXIT_INVALID          =  -1
};

struct FwCfgDmaAccess {
  uint32_t control;
  uint32_t length;
  uint64_t address;
};

void vmrun(uint64_t vmcb);
void scheduale();
void store_vmcb_guest();
// void vmrun(uint64_t vmcb_addr);
void init_vm();
void init_host();
void vmexit_handler();
void handle_ioio_vmexit();
void edit_vmcb_state(vmcb_registers reg, uint64_t value);
void inject_event(uint8_t vector, event_type_e type, bool push_error_code, uint32_t error_code);
