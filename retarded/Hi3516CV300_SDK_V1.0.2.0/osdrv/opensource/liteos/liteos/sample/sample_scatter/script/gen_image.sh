#!/bin/sh

set -e

cur_dir=`pwd`

dir_code=${cur_dir}
dir_scatter=${dir_code}/../../tools/scripts/scatter_sr
dir_ld=${dir_scatter}/../ld

platform=$1

export CFG_SCATTER_FLAG=yes
export CFG_FAST_IMAGE=yes




# step 1
cp ${dir_code}/script/wow_orignal.ld    ${dir_ld}/wow.ld   -f
cp ${dir_code}/script/scatter_orignal.ld     ${dir_ld}/scatter.ld  -f



cd ${dir_code}; make clean; make ;cd -



# step 2
cd ${dir_scatter}
chmod -R 777 *

rm -f lib_list.wow.*
rm -f lib_list.scatter.*
rm -f symbol_list.wow.*
rm -f symbol_list.scatter.*


./liblist.sh scatter   ${dir_code}/out/bin/sample  ${dir_code}/out/bin/sample.map  ${dir_scatter}    ${dir_code}/out/lib
cd -


# step 3

export CFG_FAST_IMAGE=no



cd ${dir_code}; make clean; make;cd - 


