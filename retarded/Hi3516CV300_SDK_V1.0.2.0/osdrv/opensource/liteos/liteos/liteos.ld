ENTRY(reset_vector)
INCLUDE board.ld
SECTIONS
{
    _wow_start = ABSOLUTE (.);
INCLUDE wow.ld
    __wow_end = ABSOLUTE (.);

     __fast_start = ABSOLUTE(.);
INCLUDE scatter.ld
     __fast_end = ABSOLUTE(.);

    .got ALIGN (0x4) : { . = .; *(.got.plt) *(.got) _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.); _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.); } > ram
    .rel.text : { *(.rel.text) *(.rel.text.*) *(.rel.gnu.linkonce.t*) } > ram .rela.text : { *(.rela.text) *(.rela.text.*) *(.rela.gnu.linkonce.t*) } > ram .rel.data : { *(.rel.data) *(.rel.data.*) *(.rel.gnu.linkonce.d*) } > ram .rela.data : { *(.rela.data) *(.rela.data.*) *(.rela.gnu.linkonce.d*) } > ram .rel.rodata : { *(.rel.rodata) *(.rel.rodata.*) *(.rel.gnu.linkonce.r*) } > ram .rela.rodata : { *(.rela.rodata) *(.rela.rodata.*) *(.rela.gnu.linkonce.r*) } > ram .rel.got : { *(.rel.got) } > ram .rela.got : { *(.rela.got) } > ram .rel.ctors : { *(.rel.ctors) } > ram .rela.ctors : { *(.rela.ctors) } > ram .rel.dtors : { *(.rel.dtors) } > ram .rela.dtors : { *(.rela.dtors) } > ram .rel.init : { *(.rel.init) } > ram .rela.init : { *(.rela.init) } > ram .rel.fini : { *(.rel.fini) } > ram .rela.fini : { *(.rela.fini) } > ram .rel.bss : { *(.rel.bss) } > ram .rela.bss : { *(.rela.bss) } > ram .rel.plt : { *(.rel.plt) } > ram .rela.plt : { *(.rela.plt) } > ram .rel.dyn : { *(.rel.dyn) } > ram
    .ARM.extab ALIGN (0x4) : { PROVIDE (__stext = ABSOLUTE(.)); _stext = ABSOLUTE(.) ; . = .; } > ram /DISCARD/ 0 : { *(.ARM.extab* .gnu.linkonce.armextab.*) } . = ALIGN(8); . = ALIGN(8); __exidx_start = ABSOLUTE(.); .ARM.exidx ALIGN(8) : AT ((LOADADDR (.ARM.extab) + SIZEOF (.ARM.extab) + (8) - 1) & ~ ((8) - 1)) { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > ram __exidx_end = ABSOLUTE(.); /DISCARD/ 0 : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } 
    .text ALIGN(0x1000) : AT ((LOADADDR (.ARM.exidx) + SIZEOF (.ARM.exidx) + (0x1000) - 1) & ~ ((0x1000) - 1)) { __text_start = ABSOLUTE(.); *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) *(.init) *(.glue_7) *(.glue_7t) __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); __text_end = ABSOLUTE(.); } > ram _etext = .; PROVIDE (__etext = .);
    . = (ABSOLUTE (.) + (0x1000 -1)) & ~ (0x1000 - 1);
    __text_end = ABSOLUTE (.);
    .rodata ALIGN (0x1000) : { __rodata_start = ABSOLUTE (.); . = .; *(.rodata*) *(.gnu.linkonce.r.*); __rodata_end = ABSOLUTE (.); } > ram
    . = (ABSOLUTE (.) + (0x1000 -1)) & ~ (0x1000 - 1);
    __rodata_end = ABSOLUTE (.);
    .rodata1 ALIGN (0x1000) : { __rodata1_start = ABSOLUTE (.); . = .; *(.rodata1); __rodata1_end = ABSOLUTE (.); } > ram
    . = (ABSOLUTE (.) + (0x1000 -1)) & ~ (0x1000 - 1);
    __rodata1_end = ABSOLUTE (.);
    .data ALIGN (0x4) : { __ram_data_start = ABSOLUTE (.); *(.data*) *(.data1) *(.gnu.linkonce.d.*) . = ALIGN (4); KEEP(*( SORT (.liteos.table.*))) ; . = ALIGN (4); KEEP(*( SORT (.liteos.eth.*))) ; *(.dynamic) *(.sdata*) *(.gnu.linkonce.s.*) . = ALIGN (4); *(.2ram.*) } > ram __rom_data_start = LOADADDR (.data); __ram_data_end = .; PROVIDE (__ram_data_end = .); _edata = .; PROVIDE (edata = .); PROVIDE (__rom_data_end = LOADADDR (.data) + SIZEOF(.data));
    .fini ALIGN (0x4) : { . = .; *(.fini) } > ram
    .fixup ALIGN (0x4) : { . = .; *(.fixup) } > ram
    .gcc_except_table ALIGN (0x4) : { . = .; } > ram /DISCARD/ 0 : { *(.gcc_except_table*) }
    .int_stack ALIGN (0x800) : { . = .; __int_stack_start = ABSOLUTE (.); *(.int_stack); . = ALIGN (4); } > ram
    .bss ALIGN (0x4) : {
    KEEP(*(.bss.prebss.*))
    . = ALIGN(0x4);
    __bss_start = ABSOLUTE (.); *(.scommon) *(.dynsbss) *(.tbss.*) *(.sbss*) *(.gnu.linkonce.sb.*) *(.dynbss) *(.bss*) *(.gnu.linkonce.b.*) *(COMMON) __bss_end = ABSOLUTE (.); } > ram
    .startup_stack ALIGN (0x4) : { . = .; *(.startup_stack); . = ALIGN (4); } > ram
    __heap1 = ALIGN (0x8);
    . = ALIGN(4); _end = .; PROVIDE (end = .);
}
