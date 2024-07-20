#pragma once

#include <stdint.h>

typedef enum _VMCS_FIELD
{
  VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER                      = 0x0000,
  VMCS_CTRL_POSTED_INTERRUPT_NOTIFICATION_VECTOR              = 0x0002,
  VMCS_CTRL_EPTP_INDEX                                        = 0x0004,

  //
  // control::64_bit
  //

  VMCS_CTRL_IO_BITMAP_A_ADDRESS                               = 0x2000,
  VMCS_CTRL_IO_BITMAP_B_ADDRESS                               = 0x2002,
  VMCS_CTRL_MSR_BITMAP_ADDRESS                                = 0x2004,
  VMCS_CTRL_VMEXIT_MSR_STORE_ADDRESS                          = 0x2006,
  VMCS_CTRL_VMEXIT_MSR_LOAD_ADDRESS                           = 0x2008,
  VMCS_CTRL_VMENTRY_MSR_LOAD_ADDRESS                          = 0x200A,
  VMCS_CTRL_EXECUTIVE_VMCS_POINTER                            = 0x200C,
  VMCS_CTRL_PML_ADDRESS                                       = 0x200E,
  VMCS_CTRL_TSC_OFFSET                                        = 0x2010,
  VMCS_CTRL_VIRTUAL_APIC_ADDRESS                              = 0x2012,
  VMCS_CTRL_APIC_ACCESS_ADDRESS                               = 0x2014,
  VMCS_CTRL_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS               = 0x2016,
  VMCS_CTRL_VMFUNC_CONTROLS                                   = 0x2018,
  VMCS_CTRL_EPT_POINTER                                       = 0x201A,
  VMCS_CTRL_EOI_EXIT_BITMAP_0                                 = 0x201C,
  VMCS_CTRL_EOI_EXIT_BITMAP_1                                 = 0x201E,
  VMCS_CTRL_EOI_EXIT_BITMAP_2                                 = 0x2020,
  VMCS_CTRL_EOI_EXIT_BITMAP_3                                 = 0x2022,
  VMCS_CTRL_EPT_POINTER_LIST_ADDRESS                          = 0x2024,
  VMCS_CTRL_VMREAD_BITMAP_ADDRESS                             = 0x2026,
  VMCS_CTRL_VMWRITE_BITMAP_ADDRESS                            = 0x2028,
  VMCS_CTRL_VIRTUALIZATION_EXCEPTION_INFO_ADDRESS             = 0x202A,
  VMCS_CTRL_XSS_EXITING_BITMAP                                = 0x202C,
  VMCS_CTRL_ENCLS_EXITING_BITMAP                              = 0x202E,
  VMCS_CTRL_TSC_MULTIPLIER                                    = 0x2032,

  //
  // control::32_bit
  //

  VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS                   = 0x4000,
  VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS             = 0x4002,
  VMCS_CTRL_EXCEPTION_BITMAP                                  = 0x4004,
  VMCS_CTRL_PAGEFAULT_ERROR_CODE_MASK                         = 0x4006,
  VMCS_CTRL_PAGEFAULT_ERROR_CODE_MATCH                        = 0x4008,
  VMCS_CTRL_CR3_TARGET_COUNT                                  = 0x400A,
  VMCS_CTRL_VMEXIT_CONTROLS                                   = 0x400C,
  VMCS_CTRL_VMEXIT_MSR_STORE_COUNT                            = 0x400E,
  VMCS_CTRL_VMEXIT_MSR_LOAD_COUNT                             = 0x4010,
  VMCS_CTRL_VMENTRY_CONTROLS                                  = 0x4012,
  VMCS_CTRL_VMENTRY_MSR_LOAD_COUNT                            = 0x4014,
  VMCS_CTRL_VMENTRY_INTERRUPTION_INFO                         = 0x4016,
  VMCS_CTRL_VMENTRY_EXCEPTION_ERROR_CODE                      = 0x4018,
  VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH                        = 0x401A,
  VMCS_CTRL_TPR_THRESHOLD                                     = 0x401C,
  VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS   = 0x401E,
  VMCS_CTRL_PLE_GAP                                           = 0x4020,
  VMCS_CTRL_PLE_WINDOW                                        = 0x4022,

  //
  // control::natural
  //

  VMCS_CTRL_CR0_GUEST_HOST_MASK                               = 0x6000,
  VMCS_CTRL_CR4_GUEST_HOST_MASK                               = 0x6002,
  VMCS_CTRL_CR0_READ_SHADOW                                   = 0x6004,
  VMCS_CTRL_CR4_READ_SHADOW                                   = 0x6006,
  VMCS_CTRL_CR3_TARGET_VALUE_0                                = 0x6008,
  VMCS_CTRL_CR3_TARGET_VALUE_1                                = 0x600A,
  VMCS_CTRL_CR3_TARGET_VALUE_2                                = 0x600C,
  VMCS_CTRL_CR3_TARGET_VALUE_3                                = 0x600E,

  //
  // [VMEXIT] (read-only)
  //

  //
  // vmexit::16_bit
  //

  //
  // vmexit::64_bit
  //

  VMCS_VMEXIT_GUEST_PHYSICAL_ADDRESS                          = 0x2400,

  //
  // vmexit::32_bit
  //

  VMCS_VMEXIT_INSTRUCTION_ERROR                               = 0x4400,
  VMCS_VMEXIT_REASON                                          = 0x4402,
  VMCS_VMEXIT_INTERRUPTION_INFO                               = 0x4404,
  VMCS_VMEXIT_INTERRUPTION_ERROR_CODE                         = 0x4406,
  VMCS_VMEXIT_IDT_VECTORING_INFO                              = 0x4408,
  VMCS_VMEXIT_IDT_VECTORING_ERROR_CODE                        = 0x440A,
  VMCS_VMEXIT_INSTRUCTION_LENGTH                              = 0x440C,
  VMCS_VMEXIT_INSTRUCTION_INFO                                = 0x440E,

  //
  // vmexit::natural
  //

  VMCS_VMEXIT_QUALIFICATION                                   = 0x6400,
  VMCS_VMEXIT_IO_RCX                                          = 0x6402,
  VMCS_VMEXIT_IO_RSX                                          = 0x6404,
  VMCS_VMEXIT_IO_RDI                                          = 0x6406,
  VMCS_VMEXIT_IO_RIP                                          = 0x6408,
  VMCS_VMEXIT_GUEST_LINEAR_ADDRESS                            = 0x640A,

  //
  // [GUEST]
  //

  //
  // guest::16_bit
  //

  VMCS_GUEST_ES_SELECTOR                                      = 0x0800,
  VMCS_GUEST_CS_SELECTOR                                      = 0x0802,
  VMCS_GUEST_SS_SELECTOR                                      = 0x0804,
  VMCS_GUEST_DS_SELECTOR                                      = 0x0806,
  VMCS_GUEST_FS_SELECTOR                                      = 0x0808,
  VMCS_GUEST_GS_SELECTOR                                      = 0x080A,
  VMCS_GUEST_LDTR_SELECTOR                                    = 0x080C,
  VMCS_GUEST_TR_SELECTOR                                      = 0x080E,
  VMCS_GUEST_INTERRUPT_STATUS                                 = 0x0810,
  VMCS_GUEST_PML_INDEX                                        = 0x0812,

  //
  // guest::64_bit
  //

  VMCS_GUEST_VMCS_LINK_POINTER                                = 0x2800,
  VMCS_GUEST_DEBUGCTL                                         = 0x2802,
  VMCS_GUEST_PAT                                              = 0x2804,
  VMCS_GUEST_EFER                                             = 0x2806,
  VMCS_GUEST_PERF_GLOBAL_CTRL                                 = 0x2808,
  VMCS_GUEST_PDPTE0                                           = 0x280A,
  VMCS_GUEST_PDPTE1                                           = 0x280C,
  VMCS_GUEST_PDPTE2                                           = 0x280E,
  VMCS_GUEST_PDPTE3                                           = 0x2810,

  //
  // guest::32_bit
  //

  VMCS_GUEST_ES_LIMIT                                         = 0x4800,
  VMCS_GUEST_CS_LIMIT                                         = 0x4802,
  VMCS_GUEST_SS_LIMIT                                         = 0x4804,
  VMCS_GUEST_DS_LIMIT                                         = 0x4806,
  VMCS_GUEST_FS_LIMIT                                         = 0x4808,
  VMCS_GUEST_GS_LIMIT                                         = 0x480A,
  VMCS_GUEST_LDTR_LIMIT                                       = 0x480C,
  VMCS_GUEST_TR_LIMIT                                         = 0x480E,
  VMCS_GUEST_GDTR_LIMIT                                       = 0x4810,
  VMCS_GUEST_IDTR_LIMIT                                       = 0x4812,
  VMCS_GUEST_ES_ACCESS_RIGHTS                                 = 0x4814,
  VMCS_GUEST_CS_ACCESS_RIGHTS                                 = 0x4816,
  VMCS_GUEST_SS_ACCESS_RIGHTS                                 = 0x4818,
  VMCS_GUEST_DS_ACCESS_RIGHTS                                 = 0x481A,
  VMCS_GUEST_FS_ACCESS_RIGHTS                                 = 0x481C,
  VMCS_GUEST_GS_ACCESS_RIGHTS                                 = 0x481E,
  VMCS_GUEST_LDTR_ACCESS_RIGHTS                               = 0x4820,
  VMCS_GUEST_TR_ACCESS_RIGHTS                                 = 0x4822,
  VMCS_GUEST_INTERRUPTIBILITY_STATE                           = 0x4824,
  VMCS_GUEST_ACTIVITY_STATE                                   = 0x4826,
  VMCS_GUEST_SMBASE                                           = 0x4828,
  VMCS_GUEST_SYSENTER_CS                                      = 0x482A,
  VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE                       = 0x482E,

  //
  // guest::natural
  //

  VMCS_GUEST_CR0                                              = 0x6800,
  VMCS_GUEST_CR3                                              = 0x6802,
  VMCS_GUEST_CR4                                              = 0x6804,
  VMCS_GUEST_ES_BASE                                          = 0x6806,
  VMCS_GUEST_CS_BASE                                          = 0x6808,
  VMCS_GUEST_SS_BASE                                          = 0x680A,
  VMCS_GUEST_DS_BASE                                          = 0x680C,
  VMCS_GUEST_FS_BASE                                          = 0x680E,
  VMCS_GUEST_GS_BASE                                          = 0x6810,
  VMCS_GUEST_LDTR_BASE                                        = 0x6812,
  VMCS_GUEST_TR_BASE                                          = 0x6814,
  VMCS_GUEST_GDTR_BASE                                        = 0x6816,
  VMCS_GUEST_IDTR_BASE                                        = 0x6818,
  VMCS_GUEST_DR7                                              = 0x681A,
  VMCS_GUEST_RSP                                              = 0x681C,
  VMCS_GUEST_RIP                                              = 0x681E,
  VMCS_GUEST_RFLAGS                                           = 0x6820,
  VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS                         = 0x6822,
  VMCS_GUEST_SYSENTER_ESP                                     = 0x6824,
  VMCS_GUEST_SYSENTER_EIP                                     = 0x6826,

  //
  // [HOST]
  //

  //
  // host::16_bit
  //

  VMCS_HOST_ES_SELECTOR                                       = 0x0C00,
  VMCS_HOST_CS_SELECTOR                                       = 0x0C02,
  VMCS_HOST_SS_SELECTOR                                       = 0x0C04,
  VMCS_HOST_DS_SELECTOR                                       = 0x0C06,
  VMCS_HOST_FS_SELECTOR                                       = 0x0C08,
  VMCS_HOST_GS_SELECTOR                                       = 0x0C0A,
  VMCS_HOST_TR_SELECTOR                                       = 0x0C0C,

  //
  // host::64_bit
  //

  VMCS_HOST_PAT                                               = 0x2C00,
  VMCS_HOST_EFER                                              = 0x2C02,
  VMCS_HOST_PERF_GLOBAL_CTRL                                  = 0x2C04,

  //
  // host::32_bit
  //

  VMCS_HOST_SYSENTER_CS                                       = 0x4C00,

  //
  // host::natural
  //

  VMCS_HOST_CR0                                               = 0x6C00,
  VMCS_HOST_CR3                                               = 0x6C02,
  VMCS_HOST_CR4                                               = 0x6C04,
  VMCS_HOST_FS_BASE                                           = 0x6C06,
  VMCS_HOST_GS_BASE                                           = 0x6C08,
  VMCS_HOST_TR_BASE                                           = 0x6C0A,
  VMCS_HOST_GDTR_BASE                                         = 0x6C0C,
  VMCS_HOST_IDTR_BASE                                         = 0x6C0E,
  VMCS_HOST_SYSENTER_ESP                                      = 0x6C10,
  VMCS_HOST_SYSENTER_EIP                                      = 0x6C12,
  VMCS_HOST_RSP                                               = 0x6C14,
  VMCS_HOST_RIP                                               = 0x6C16,
} VMCS_FIELD;

void vmwrite(VMCS_FIELD field, uint64_t data);
uint64_t vmread(VMCS_FIELD field);
void vmxon();
void vmptrld(uint64_t vmcs_ptr);

