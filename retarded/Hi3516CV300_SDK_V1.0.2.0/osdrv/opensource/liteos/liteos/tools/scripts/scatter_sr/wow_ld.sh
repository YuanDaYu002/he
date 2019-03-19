#!/bin/bash

ld_dir=$1
root_dir=$2

# add include
#sed -i '/wow.ld/d' $root_dir/liteos.ld
#sed -i "/__wow_bss_start/a\INCLUDE tools\/scripts\/ld\/wow.ld" $root_dir/liteos.ld
sed -i '/\.rodata/d' $ld_dir/wow.ld
sed -i '/\.constdata/d' $ld_dir/wow.ld
sed -i '/\.text/d' $ld_dir/wow.ld
sed -i '/\.data/d' $ld_dir/wow.ld
sed -i '/\.bss/d' $ld_dir/wow.ld
sed -i '/\COMMON/d' $ld_dir/wow.ld
sed -i '/\.int_stack/d' $ld_dir/wow.ld

# wow_rodata
sed -i '/.wow_rodata ALIGN (0x4): /a\wow.O(.rodata*);' $ld_dir/wow.ld

# wow_constdata
sed -i '/.wow_constdata ALIGN (0x4): /a\wow.O(.constdata*);' $ld_dir/wow.ld

# wow_text
sed -i '/.wow_text ALIGN (0x4): /a\wow.O(.text*);' $ld_dir/wow.ld

# wow_data
sed -i '/.wow_data ALIGN (0x4): /a\ . = ALIGN (4); KEEP(wow.O( SORT (.liteos.table.*.wow.*))) ; . = ALIGN (4); wow.O(.data*);' $ld_dir/wow.ld

# wow_bss
sed -i '/.wow_bss ALIGN (0x4): /a\wow.O(.bss*);' $ld_dir/wow.ld
sed -i '/.wow_bss ALIGN (0x4): /a\wow.O(COMMON);' $ld_dir/wow.ld
sed -i '/.wow_bss ALIGN (0x4): /a\wow.O(.int_stack*);wow.O(.startup_stack*);' $ld_dir/wow.ld
