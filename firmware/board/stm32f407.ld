ENTRY(Reset_Handler)
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  CCM (rw) : ORIGIN = 0x10000000, LENGTH = 64K
}
_estack = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS
{
  .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) } > FLASH
  .text : {
    *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini))
    . = ALIGN(4);
  } > FLASH
  .ARM.extab : { *(.ARM.extab*) } > FLASH
  .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH
  .preinit_array : {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > FLASH
  .init_array : {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > FLASH
  .fini_array : {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > FLASH
  _sidata = LOADADDR(.data);
  .data : { . = ALIGN(4); _sdata = .; *(.data*) . = ALIGN(4); _edata = .; } > RAM AT > FLASH
  .bss (NOLOAD) : {
    . = ALIGN(8); _sbss = .; *(.bss*) *(COMMON)
    . = ALIGN(8); _ebss = .;
  } > RAM
  .dma (NOLOAD) : {
    . = ALIGN(32); __dma_start__ = .; *(.dma*)
    . = ALIGN(32); __dma_end__ = .;
  } > RAM
  . = ALIGN(8);
  end = .; _end = .;
  __ram_used_end__ = .;
  /DISCARD/ : { *(.note.GNU-stack) }
}
ASSERT(__ram_used_end__ <= _estack - 4096, "SRAM exceeds budget: reserve 4KB for MSP")
ASSERT(__dma_start__ >= ORIGIN(RAM) && __dma_end__ <= _estack, "DMA must be in SRAM")
