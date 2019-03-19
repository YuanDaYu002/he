.fast_rodata ALIGN (0x1000): { __fast_rodata_start = ABSOLUTE(.); . = .;
}

.fast_constdata ALIGN (0x4): { __fast_constdata_start = ABSOLUTE(.); . = .;
}

.fast_init_array ALIGN (0x4): {
. = ALIGN (4); __init_array_start__ = ABSOLUTE (.); KEEP (*(SORT (.init_array.*))) KEEP (*(SORT (.init_array))) __init_array_end__ = ABSOLUTE (.);
}

. = (ABSOLUTE (.) + (0x1000 - 1)) & ~ (0x1000 - 1);
__fast_rodata_end = ABSOLUTE(.);

.fast_text ALIGN (0x1000): { __fast_text_start = ABSOLUTE(.); . = .;
}

. = (ABSOLUTE (.) + (0x1000 - 1)) & ~ (0x1000 - 1);
__fast_text_end = ABSOLUTE (.);

.fast_data ALIGN (0x4): { __fast_data_start = ABSOLUTE(.); . = .;
}

. = (ABSOLUTE (.) + (0x1000 - 1)) & ~ (0x1000 - 1);
__fast_data_end = ABSOLUTE(.);
