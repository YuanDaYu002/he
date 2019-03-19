#!/bin/bash

byte_align_low=7FF
byte_align_low=`echo "obase=10;ibase=16;${byte_align_low}"|bc`

text_flag=`readelf -S $1 | grep "\.text" | awk '{print $1}'`
mem_flag=`readelf -S $1 | grep "\.rom_vectors" | awk '{print $1}'`

if [ $mem_flag = "[" ]; then
	mem_addr=`readelf -S $1 | grep "\.rom_vectors" | awk '{print $5}'`
else
	mem_addr=`readelf -S $1 | grep "\.rom_vectors" | awk '{print $4}'`
fi
if [ $text_flag = "[" ]; then
	text_addr=`readelf -S $1 | grep "\.text" | awk '{print $5}'`
else
	text_addr=`readelf -S $1 | grep "\.text" | awk '{print $4}'`
fi

let scatter_size=(16#$text_addr-16#$mem_addr)
byte_align_low=800
byte_align_high=FFF000
byte_align_high=`echo "obase=10;ibase=16;${byte_align_high}"|bc`
let byte_align=(16#$byte_align_low+$byte_align_high)
let scatter=($scatter_size'&'$byte_align)
scatter_size=`echo "obase=16;${scatter}"|bc`

echo "###### ###### ##### ##### ##### ##### #####"
echo "#######Calculate the size of scatter#######"
echo "the size is 0x$scatter_size"
echo "###################end#####################"
