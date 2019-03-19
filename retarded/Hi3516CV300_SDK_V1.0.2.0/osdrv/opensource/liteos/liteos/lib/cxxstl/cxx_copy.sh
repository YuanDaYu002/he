#!/bin/bash
declare current_version=20170512
declare TEMP=$1
declare COMPILER_LONG_NAME=${TEMP%-*}
declare COMPILER_NAME=${TEMP#*-}
declare COMPILER_NAME=${COMPILER_NAME%-li*}
declare ISAARCH64=`echo $1 | grep -o 'aarch64'`
declare OBJ_DIR="/";
declare OBJ_DIR_TEMP=""
declare VERSION_NUM=$2
declare LIB=$3
declare CUR_DIR=$PWD
declare FIND_FLAG=0

function get_compiler_path()
{
    local SYSTEM=`uname -s`
    local user_gcc=${TEMP}gcc
    local gcc_install_path=`which ${user_gcc}`
    if [ $SYSTEM = "Linux" ] ; then
        gcc_install_path=${gcc_install_path/\/bin\/${user_gcc}/}
    else
        gcc_install_path=${gcc_install_path/\/bin\/${user_gcc}.exe/}
    fi
    OBJ_DIR=$gcc_install_path
}

function check_compiler_version()
{
    if [ "$COMPILER_LONG_NAME" == "arm-himix100-linux" ] || [ "$COMPILER_LONG_NAME" == "aarch64-himix100-linux" ]; then
        COMPILE_NAME=${TEMP%-}
        #check gcc version
        local user_gcc=${TEMP}gcc
        version_str=Huaweiliteos_v100_
        gccver=`${user_gcc} --version 2>&1`
        echo $gccver
        echo "$gccver" |grep -q $version_str
        if [ $? -ne 0 ]
        then
            echo -e "\033[31merror: The compiler version should be $version_str${current_version}\033[0m"
            exit -1
        else
            ver_num=`echo $gccver | grep -o '[0-9]\{8\}'`
            ver_num=($ver_num)
            if [ $current_version -gt $ver_num ] ; then
                echo -e "\033[31merror: You are using an old compiler version, and should update to $version_str${current_version} or newer\033[0m"
                exit -2
            elif [ $current_version -eq $ver_num ] ; then
                echo "Compiler Version OK"
            else
                echo "Old Project"
            fi
        fi
    else
        COMPILE_NAME=${TEMP%x*}
    fi
}

function lib_patch()
{
    local COMPILE_NAME_TEMP=$1
    ############### patch the modify of c++ headers #############
    pushd $CUR_DIR/lib
    if [ "$COMPILE_NAME_TEMP" == "aarch64-himix100-linux" ];then
        patch -p1 < $CUR_DIR/lib/cxxstl/cxx_huaweiliteos.patch
    elif [ "$COMPILE_NAME_TEMP" == "arm-himix100-linux" ];then
        patch -p1 < $CUR_DIR/lib/cxxstl/cxx_huaweiliteos.patch
    else
        echo "patch error!!!"
    fi
    popd
}

#check_compiler_version

if [[ $ISAARCH64 == "aarch64" ]];then

    if [ "$COMPILER_LONG_NAME" == "aarch64-himix100-linux" ];then
        get_compiler_path
        #copy gcc lib
        GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILER_LONG_NAME/$VERSION_NUM
        mkdir -p $CUR_DIR/lib/armv8_arm64
        cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/armv8_arm64

        #copy cxx lib
        CXX_HEAD_DIR=$CUR_DIR/lib/cxxstl
        CXX_LIB_DIR="$OBJ_DIR"/$COMPILER_LONG_NAME/lib64
        cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/armv8_arm64

        cp -rf "$OBJ_DIR"/$COMPILER_LONG_NAME/include/c++ $CXX_HEAD_DIR
        lib_patch $COMPILER_LONG_NAME

    fi

    exit 0

else

    if [ "$COMPILER_LONG_NAME" == "arm-himix100-linux" ]; then
        get_compiler_path

        #copy gcc lib
        GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILER_LONG_NAME/$VERSION_NUM/a7_softfp_neon-vfpv4
        mkdir -p $CUR_DIR/lib/a7_softfp_neon-vfpv4
        cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/a7_softfp_neon-vfpv4

        GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILER_LONG_NAME/$VERSION_NUM/a17_softfp_neon-vfpv4
        mkdir -p $CUR_DIR/lib/a17_softfp_neon-vfpv4
        cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/a17_softfp_neon-vfpv4

        GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILER_LONG_NAME/$VERSION_NUM/armv5te_arm9_soft
        mkdir -p $CUR_DIR/lib/armv5te_arm9_soft
        cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/armv5te_arm9_soft

        #copy cxx lib
        CXX_HEAD_DIR=$CUR_DIR/lib/cxxstl
        CXX_LIB_DIR="$OBJ_DIR"/$COMPILER_LONG_NAME/lib/a7_softfp_neon-vfpv4
        cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/a7_softfp_neon-vfpv4

        CXX_LIB_DIR="$OBJ_DIR"/$COMPILER_LONG_NAME/lib/a17_softfp_neon-vfpv4
        cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/a17_softfp_neon-vfpv4

        CXX_LIB_DIR="$OBJ_DIR"/$COMPILER_LONG_NAME/lib/armv5te_arm9_soft
        cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/armv5te_arm9_soft

        #cxx header
        cp -rf "$OBJ_DIR"/$COMPILER_LONG_NAME/include/c++ $CXX_HEAD_DIR
        lib_patch $COMPILER_LONG_NAME
        exit 0
    fi
fi

# compiler is not arm-himix100-linux or arm-himix200-linux
echo -e "The compiler version should be arm-himix100-linux or arm-himix200-linux"
exit -1

#check if compiler had been changed, then delete old files
last_compiler_path="./out/.last_compiler"
if [ -f $last_compiler_path ]; then
    LAST_COMPILER=$(cat $last_compiler_path)
    if [[ $LAST_COMPILER != $COMPILE_NAME ]];then
        rm -rf ./lib/cxxstl/c++
        rm -rf ./lib/cxxstl/gccinclude
        rm -rf ./lib/cxxstl/gdbinclude
        rm -rf ./lib/a7_softfp_neon-vfpv4/*.a
        rm -rf ./lib/a17_softfp_neon-vfpv4/*.a
        rm -rf ./lib/armv5te_arm9_soft/*.a
    fi
else
    LAST_COMPILER=$COMPILE_NAME
fi
#update .last_compiler
mkdir -p out
echo $COMPILE_NAME>$last_compiler_path

#FILE_NUM=`find $CUR_DIR/lib/cxxstl -name *.h | wc -l`
FILE_NUM=`find $CUR_DIR/lib/cxxstl/c++/$VERSION_NUM -name *.h | wc -l`

if [[ $FILE_NUM -gt 0 ]];then
	echo "do not copy the header again!!!!"
	exit
fi

######### usb special characters to keep the blank ########
PATH_NO_BLANK="$(echo $PATH | sed 's/ /_-_/g')"

######### find the path of compiler ############
OLD_IFS="$IFS"
IFS=":"
arr=($PATH_NO_BLANK)
IFS="$OLD_IFS"

for s in ${arr[@]}
do
PATH_TEMP="$(echo $s | sed s'/_-_/ /g')"
if [[ "$PATH_TEMP" =~ .*$COMPILE_NAME.* ]];then
	OBJ_DIR_TEMP=$s
	break
fi
done

########## find the name of compiler ###########
OLD_IFS="$IFS"
IFS="/"
arr=($OBJ_DIR_TEMP)
IFS="$OLD_IFS"

OBJ_DIR="/"
for s in ${arr[@]}
do
FILE_NAME="$(echo $s | sed s'/_-_/ /g')"
OBJ_DIR="$OBJ_DIR""$FILE_NAME"
if [[ "$FILE_NAME" =~ .*$COMPILE_NAME.* ]];then
        COMPILE_NAME_TEMP=$FILE_NAME
	FIND_FLAG=1
        break
fi
OBJ_DIR="$OBJ_DIR""/"
done

if [ "$FIND_FLAG" == 0 ];then
	echo "find the compiler error!!!!"
	echo "+++++you do not have the compiler!!!+++++"
	exit
fi

FIRST_C=`echo ${COMPILE_NAME_TEMP:0:1}`
if [ "$FIRST_C" == "." ];then
	COMPILE_NAME_TEMP=`echo ${COMPILE_NAME_TEMP:1}`
fi
ls "$OBJ_DIR" > $CUR_DIR/dir_list.txt
while read line
do
        if [[ $line =~ .*$COMPILE_NAME_TEMP.* ]];then
		COMPILE_NAME_TEMP=$line
		break
	fi
done < $CUR_DIR/dir_list.txt
rm -rf $CUR_DIR/dir_list.txt

############## copy the lib ####################
GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILE_NAME_TEMP/$VERSION_NUM/a7_softfp_neon-vfpv4
mkdir -p $CUR_DIR/lib/a7_softfp_neon-vfpv4
cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/a7_softfp_neon-vfpv4

GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILE_NAME_TEMP/$VERSION_NUM/a17_softfp_neon-vfpv4
mkdir -p $CUR_DIR/lib/a17_softfp_neon-vfpv4
cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/a17_softfp_neon-vfpv4

GCC_LIB_DIR="$OBJ_DIR"/lib/gcc/$COMPILE_NAME_TEMP/$VERSION_NUM/armv5te_arm9_soft
mkdir -p $CUR_DIR/lib/armv5te_arm9_soft
cp "$GCC_LIB_DIR"/libgcc.a "$GCC_LIB_DIR"/libgcc_eh.a "$GCC_LIB_DIR"/libgcov.a $CUR_DIR/lib/armv5te_arm9_soft

if [ $LIB == "CXX" ]
then
CXX_LIB_DIR="$OBJ_DIR"/$COMPILE_NAME_TEMP/lib/a7_softfp_neon-vfpv4
cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/a7_softfp_neon-vfpv4

CXX_LIB_DIR="$OBJ_DIR"/$COMPILE_NAME_TEMP/lib/a17_softfp_neon-vfpv4
cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/a17_softfp_neon-vfpv4

CXX_LIB_DIR="$OBJ_DIR"/$COMPILE_NAME_TEMP/lib/armv5te_arm9_soft
cp "$CXX_LIB_DIR"/libstdc++.a "$CXX_LIB_DIR"/libsupc++.a $CUR_DIR/lib/armv5te_arm9_soft

CXX_HEAD_DIR=$CUR_DIR/lib/cxxstl
rm -rf $CXX_HEAD_DIR/c++ $CXX_HEAD_DIR/gdbinclude $CXX_HEAD_DIR/gccinclude
mkdir -p $CXX_HEAD_DIR/gdbinclude

############# copy c++ headers ####################
cp -rf "$OBJ_DIR"/$COMPILE_NAME_TEMP/include/c++ $CXX_HEAD_DIR
cp "$OBJ_DIR"/lib/gcc/$COMPILE_NAME_TEMP/$VERSION_NUM/include/*.h $CXX_HEAD_DIR/gdbinclude

if [ "$COMPILE_NAME_TEMP" != "arm-huaweiliteos-linux-androideabi" ];then
    mkdir -p $CXX_HEAD_DIR/gccinclude
    mkdir -p $CXX_HEAD_DIR/gccinclude/bits
    mkdir -p $CXX_HEAD_DIR/gccinclude/sys

    cp "$OBJ_DIR"/target/usr/include/*.h $CXX_HEAD_DIR/gccinclude
    cp "$OBJ_DIR"/target/usr/include/bits/*.h $CXX_HEAD_DIR/gccinclude/bits
    cp "$OBJ_DIR"/target/usr/include/sys/*.h $CXX_HEAD_DIR/gccinclude/sys
    ############# delete the headers which conflict LiteOS header ##########
    DEL_FILE_LIST=$CUR_DIR/lib/cxxstl/del_file.txt
    while read line
    do
        rm $CUR_DIR/lib/cxxstl/$line
    done < $DEL_FILE_LIST
fi

############### patch the modify of c++ headers #############
lib_patch $COMPILE_NAME_TEMP

fi #if [ $LIB == "CXX"]
