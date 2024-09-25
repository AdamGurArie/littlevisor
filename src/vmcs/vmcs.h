#pragma once

//#include "../vm_drivers/ide.h"
#include "../vm_drivers/ata_pio.h"
#include <cstdint>

#define VMCB_STATESAVE_AREA_OFFSET 0x3FF
#define MEMORY_SPACE_PER_VM 20000000

#define MEM_SIZE_PER_VM 0x4000000
#define MEM_SIZE_COREBOOT 0x1000000

static uint32_t list_of_ports_to_intercept[] = {0x1f7};


enum vmcb_registers {
  RAX,
  RCX,
  RBX
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
  uint64_t np_enable;
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
  uint64_t reserved_12;
  uint64_t avic_logical_table_ptr;
  uint64_t avic_physical_table_ptr;
  uint64_t reserved_13;
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
  uint8_t reserved_1[42];
  uint8_t cpl;
  uint32_t reserved_2;
  uint64_t efer;
  uint8_t reserved_3[7];
  uint64_t perf_ctl0;
  uint64_t perf_ctr0;
  uint64_t perf_ctl1;
  uint64_t perf_ctr1;
  uint64_t perf_ctl2;
  uint64_t perf_ctr2;
  uint64_t perf_ctl3;
  uint64_t perf_ctr3;
  uint64_t perf_ctl4;
  uint64_t perf_ctr4;
  uint64_t perf_ctl5;
  uint64_t perf_ctr5;
  uint64_t cr4;
  uint64_t cr3;
  uint64_t cr0;
  uint64_t dr7;
  uint64_t dr6;
  uint64_t rflags;
  uint64_t rip;
  uint8_t reserved_4[63];
  uint64_t instr_retired_ctr;
  uint64_t perf_ctr_global_sts;
  uint64_t perf_ctr_global_ctl;
  uint8_t reserved_5[3];
  uint64_t rsp;
  uint64_t s_cet;
  uint64_t ssp;
  uint64_t isst_addr;
  uint64_t rax;
  uint64_t star;
  uint64_t lstar;
  uint64_t cstar;
  uint64_t sfmask;
  uint64_t kernelgsbase;
  uint64_t sysenter_cs;
  uint64_t sysenter_esp;
  uint64_t sysenter_eip;
  uint64_t cr2;
  uint8_t reserved_6[31];
  uint64_t g_pat;
  uint64_t dbgctl;
  uint64_t br_from;
  uint64_t br_to;
  uint64_t lastexepfrom;
  uint64_t lastexcpto;
  uint64_t dbgexinctl;
  uint8_t reserved_7[72];
  uint64_t spec_ctrl;
  uint8_t reserved_8[904];
  uint8_t lbr_stack_from_to[256];
  uint64_t lbr_select;
  uint64_t ibs_fetch_ctl;
  uint64_t ibs_fetch_linaddr;
  uint64_t ibs_op_ctl;
  uint64_t ibs_op_rip;
  uint64_t ibs_of_data;
  uint64_t ibs_op_data2;
  uint64_t ibs_op_data3;
  uint64_t ibs_dc_linaddr;
  uint64_t bs_ibstgt_rip; 
  uint64_t ic_ibs_extd_ctl;
} __attribute__((packed));

struct vmcb {
  vmcb_control control;
  vmcb_state_save_area state_save_area;
} __attribute__((packed));

struct context {
  vmcb_state_save_area guest;
  uint64_t guest_cr3;
  ata_pio_device ata_device;
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

void vmrun(uint64_t vmcb_addr);
void scheduale();
void store_vmcb_guest();
void vmrun();
void init_vm();
void vmexit_handler();
void handle_ioio_vmexit();
void edit_vmcb_state(vmcb_registers reg, uint64_t value);
void inject_event(uint8_t vector, event_type_e type, bool push_error_code, uint32_t error_code);
