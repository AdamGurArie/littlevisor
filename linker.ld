ENTRY(_Z6_startv)

PHDRS {
  headers PT_PHDR PHDRS;
  text    PT_LOAD FILEHDR PHDRS;
  rodata  PT_LOAD;
  data    PT_LOAD;
  dynamic PT_DYNAMIC;
}

SECTIONS
{
    . = 0xffffffff80000000 + SIZEOF_HEADERS; /* 2 MB in the higher half */

    .text :
    {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata :
    {
      *(.rodata .rodata*)
    }

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data :
    {
        *(.data .data.*)
        KEEP(*(.requests_start_marker))
        KEEP(*(.requests))
        KEEP(*(.requests_end_marker))
    } :data

    .dynamic : {
      *(.dynamic)
    } :data :dynamic

    .bss :
    {
        *(COMMON)
        *(.bss .bss.*)
    }

   /DISCARD/ : {
    *(.eh_frame*)
    *(.note .note.*)
    *(.interp)
   }

   . = 0x6000000;

   .fs_data_section :
   {
    KEEP(*(.data))
   }
}
