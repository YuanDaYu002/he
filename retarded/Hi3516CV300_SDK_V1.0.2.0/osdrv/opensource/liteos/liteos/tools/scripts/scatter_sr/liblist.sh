#!/bin/bash

target=$1

# image path
image_file=$2

# map file path
map_file=$3

# scripts dir
scripts_dir=$4

shift 4

# libs dir
outlib_dirs=$@

# $4    TYPE
# $5    BIND
# $6    VIS
# $7    NDX
# $8    NAME

nm -t decimal $image_file | sort -k1,1 |

awk -F  " " ' {
    if (""$1""!~/U/ && ""$2""!~/a/) {
        printf("%d %s %s\n", ""$1"", ""$2"", ""$3"");
    }
}
'| awk -F " " ' {
    if (""$2""~/T/ || ""$2""~/t/ || ""$2""~/D/ || ""$2""~/W/ || ""$2""~/R/) {
        if ($3 != "$a") {
            if ($3 != "$d") {
                if ($3 != "$t") {
                    print ""$3"";
                }
            }
        }
    }
}
'| cat > $scripts_dir/symbol_list.$target

# python $scripts_dir/make_liblist.py $scripts_dir/symbol_list.$target $outlib_dirs > $scripts_dir/lib_list.$target

tmp_dir=$scripts_dir/tmp
if [[ ! -d $tmp_dir ]]; then
	mkdir $scripts_dir/tmp
fi

sed -n '/LOAD\ \//p' $map_file | sed 's/LOAD\ //g' > $tmp_dir/lib_list1.txt
python $scripts_dir/make_liblist.py $scripts_dir/symbol_list.$target $outlib_dirs > $tmp_dir/lib_list2.txt

grep -vxFf $tmp_dir/lib_list1.txt $tmp_dir/lib_list2.txt > $tmp_dir/lib_list3.txt
grep -vxFf $tmp_dir/lib_list3.txt $tmp_dir/lib_list2.txt > $tmp_dir/lib_list4.txt

sort -u $tmp_dir/lib_list4.txt > $tmp_dir/lib_list.txt
cp -f $tmp_dir/lib_list.txt  $scripts_dir/lib_list.$target

rm -rf $tmp_dir
