#!/bin/bash
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

check_program_exists() {
    local ret='0'
    command -v $1 >/dev/null 2>&1 || { local ret='1'; }
    if [ "$ret" -ne 0 ]; then
        return 1
    fi
    return 0
}

echo "[ICUData]++++++++++++++++++Pkg icu data start.++++++++++++++++++++++"
date +%F' '%H:%M:%S
echo $@

script_path=$(cd $(dirname $0); pwd)
arkui_root_path="$script_path/../../../.."

if ! check_program_exists realpath; then
    echo "Command \"realpath\" not found, please install according to the system."
    exit 1
fi

if ! arkui_root_path=$(realpath "$arkui_root_path"); then
    echo "Cannot find real path for arkui_root_path '$arkui_root_path'."
    exit 1
fi

icu_data_root_path="$script_path/.."
icu_source_path="$arkui_root_path/third_party/icu/icu4c"
icu_data_filter_file="$icu_data_root_path/filters/data_filter.json"
out_put_root_path="out/arkui-cross_android_arm64"
icu_bin_root_out_dir="out/arkui-cross_android_arm64/clang_x64"
tool_lib_dir="global/global"
i18n_lib_dir="thirdparty/icu"
tool_bin_dir="global/global"
target_os="android"
host_os="linux"

while getopts "o:b:l:i:n:t:c:h" arg;
do
    case "${arg}" in
        "o")
            out_put_root_path=${OPTARG}
        ;;
        "n")
            icu_bin_root_out_dir=${OPTARG}
        ;;
        "b")
            tool_bin_dir=${OPTARG}
        ;;
        "l")
            tool_lib_dir=${OPTARG}
        ;;
        "i")
            i18n_lib_dir=${OPTARG}
        ;;
        "t")
            target_os=${OPTARG}
        ;;
        "c")
            host_os=${OPTARG}
        ;;
        "h")
            echo "help"
            exit 0
        ;;
    ?)
        echo "unkonw argument"
        exit 1
        ;;
    esac
done

out_put_root_path="$arkui_root_path/$out_put_root_path"
if ! out_put_root_path=$(realpath "$out_put_root_path"); then
    echo "Cannot find real path for out_put_root_path '$out_put_root_path'."
    exit 1
fi

icu_bin_root_out_dir="$arkui_root_path/$icu_bin_root_out_dir"
if ! icu_bin_root_out_dir=$(realpath "$icu_bin_root_out_dir"); then
    echo "Cannot find real path for icu_bin_root_out_dir '$icu_bin_root_out_dir'."
    exit 1
fi

tool_bin_dir="$icu_bin_root_out_dir/$tool_bin_dir"
tool_lib_dir="$icu_bin_root_out_dir/$tool_lib_dir"
i18n_lib_dir="$icu_bin_root_out_dir/$i18n_lib_dir"
res_out_root_dir="$out_put_root_path/icu_data"
host_build_dir="$res_out_root_dir/host"
pkg_inc_path=$res_out_root_dir/icu_pkgdata.inc
icudefs_mk_path=$host_build_dir/icudefs.mk
host_os_build_type="linux"

cd "$out_put_root_path"
rm -rf "$out_put_root_path/icu_data"
mkdir -p "$res_out_root_dir/out/lib"
mkdir -p "$res_out_root_dir/res"

config_for_host() {
    echo "[ICUData] config host start."
    mkdir -p "$host_build_dir"
    cd "$host_build_dir"
    if [ "$host_os"X = "linux"X ]; then
        host_os_build_type="Linux"
    elif [ "$host_os"X = "mac"X ]; then
        host_os_build_type="MacOSX"
    else
        echo "Unsupported this system: $host_os"
        return 1
    fi

    if [ -n "$icu_data_filter_file" ]; then
        if ICU_DATA_FILTER_FILE=$(realpath "$icu_data_filter_file"); then
            export ICU_DATA_FILTER_FILE=$ICU_DATA_FILTER_FILE
        fi
    fi

    $icu_source_path/source/runConfigureICU $host_os_build_type -q
    return 0
}

gen_pkgdata_inc_for_linux_host() {
    top_srcdir=$icu_source_path/source
    top_builddir=$host_build_dir

    GENCCODE_ASSEMBLY=`grep -Po '(?<=GENCCODE_ASSEMBLY = ).*' $icudefs_mk_path`   
    SO="so"
    SOBJ="so"
    A="a"
    LIBPREFIX="lib"
    FINAL_SO_TARGET="."
    RPATH_FLAGS=""
    BIR_LDFLAGS="-Wl,-Bsymbolic"
    AR=`grep "\<AR \= " $icudefs_mk_path | awk -F " " '{print $3'}`
    ARFLAGS=`grep -Po '(?<=ARFLAGS = ).*' $icudefs_mk_path`
    RANLIB=`grep -Po '(?<=RANLIB = ).*' $icudefs_mk_path`
    LDFLAGSICUDT=""
    LD_SONAME=""
    INSTALL=""

    SHAREDLIBCPPFLAGS="-DPIC"
    SHAREDLIBCFLAGS="-fPIC"

        THREADSCPPFLAGS="-D_REENTRANT"
        THREADSCFLAGS=""
        LD_SOOPTIONS="-Wl,-Bsymbolic"

        CC=`grep -Po '(?<=CC = ).*' $icudefs_mk_path`
        CPPFLAGS=`grep -Po '(?<=CPPFLAGS = ).*' $icudefs_mk_path`
        DEFS=`grep -Po '(?<=DEFS = ).*' $icudefs_mk_path`
        CFLAGS=`grep -Po '(?<=CFLAGS = ).*' $icudefs_mk_path`
        #LDFLAGS=`grep -Po '(?<=LDFLAGS = ).*' $icudefs_mk_path`
        LDFLAGS=""

    COMPILE="$CC $CPPFLAGS $DEFS $CFLAGS -c"
        COMPILE=${COMPILE/\$\(THREADSCPPFLAGS\)/$THREADSCPPFLAGS}
        COMPILE=${COMPILE/\$\(THREADSCFLAGS\)/$THREADSCFLAGS}

    SHLIB="$CC $CFLAGS $LDFLAGS -shared $LD_SOOPTIONS"
        SHLIB=${SHLIB/\$\(THREADSCFLAGS\)/$THREADSCFLAGS}

    echo "GENCCODE_ASSEMBLY_TYPE=$GENCCODE_ASSEMBLY" >> $pkg_inc_path
    echo "SO=$SO" >> $pkg_inc_path
    echo "SOBJ=$SOBJ" >> $pkg_inc_path
    echo "A=$A" >> $pkg_inc_path
    echo "LIBPREFIX=$LIBPREFIX" >> $pkg_inc_path
    echo "LIB_EXT_ORDER=$FINAL_SO_TARGET" >> $pkg_inc_path
    echo "COMPILE=$COMPILE" >> $pkg_inc_path
    echo "LIBFLAGS="-I$top_srcdir/common -I$top_builddir/common $SHAREDLIBCPPFLAGS $SHAREDLIBCFLAGS"" >> $pkg_inc_path
    echo "GENLIB=$SHLIB" >> $pkg_inc_path
    echo "LDICUDTFLAGS=$LDFLAGSICUDT" >> $pkg_inc_path
    echo "LD_SONAME=$LD_SONAME" >> $pkg_inc_path
    echo "RPATH_FLAGS=$RPATH_FLAGS" >> $pkg_inc_path
    echo "BIR_LDFLAGS=$BIR_LDFLAGS" >> $pkg_inc_path
    echo "AR=$AR" >> $pkg_inc_path
    echo "ARFLAGS=$ARFLAGS" >> $pkg_inc_path
    echo "RANLIB=$RANLIB" >> $pkg_inc_path
    echo "INSTALL_CMD=$INSTALL" >> $pkg_inc_path
}

gen_pkgdata_inc_for_mac_host() {
    top_srcdir=$icu_source_path/source
    top_builddir=$host_build_dir

    GENCCODE_ASSEMBLY="-a gcc-darwin"
    SO="dylib"
    SOBJ="dylib"
    A="a"
    LIBPREFIX="lib"
    FINAL_SO_TARGET="."
    RPATH_FLAGS=""
    BIR_LDFLAGS=""
    AR="ar"
    ARFLAGS="r -c"
    RANLIB="ranlib"

    LDFLAGSICUDT=""
    LD_SONAME=""
    INSTALL=""

    SHAREDLIBCPPFLAGS="-dynamic"
    SHAREDLIBCFLAGS=""

        CC=`grep "\<CC \= " $icudefs_mk_path | awk -F "=" '{print $2'}`

    COMPILE="$CC -DU_ATTRIBUTE_DEPRECATED=   -DU_HAVE_STRTOD_L=1 -DU_HAVE_XLOCALE_H=1 -DU_HAVE_STRING_VIEW=1  -O2  -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings  -Qunused-arguments -Wno-parentheses-equality -fno-common -c"
    SHLIB="$CC -dynamiclib -dynamic -O2  -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings  -Qunused-arguments -Wno-parentheses-equality "
    
    echo "GENCCODE_ASSEMBLY_TYPE=$GENCCODE_ASSEMBLY" >> $pkg_inc_path
    echo "SO=$SO" >> $pkg_inc_path
    echo "SOBJ=$SOBJ" >> $pkg_inc_path
    echo "A=$A" >> $pkg_inc_path
    echo "LIBPREFIX=$LIBPREFIX" >> $pkg_inc_path
    echo "LIB_EXT_ORDER=$FINAL_SO_TARGET" >> $pkg_inc_path
    echo "COMPILE=$COMPILE" >> $pkg_inc_path
    echo "LIBFLAGS="-I$top_srcdir/common -I$top_builddir/common $SHAREDLIBCPPFLAGS $SHAREDLIBCFLAGS"" >> $pkg_inc_path
    echo "GENLIB=$SHLIB" >> $pkg_inc_path
    echo "LDICUDTFLAGS=$LDFLAGSICUDT" >> $pkg_inc_path
    echo "LD_SONAME=$LD_SONAME" >> $pkg_inc_path
    echo "RPATH_FLAGS=$RPATH_FLAGS" >> $pkg_inc_path
    echo "BIR_LDFLAGS=$BIR_LDFLAGS" >> $pkg_inc_path
    echo "AR=$AR" >> $pkg_inc_path
    echo "ARFLAGS=$ARFLAGS" >> $pkg_inc_path
    echo "RANLIB=$RANLIB" >> $pkg_inc_path
    echo "INSTALL_CMD=$INSTALL" >> $pkg_inc_path
}

gen_pkgdata_inc() {   
    if [ "$host_os"X = "linux"X ]; then
        # mh-linux
        gen_pkgdata_inc_for_linux_host
    elif [ "$host_os"X = "mac"X ]; then
        # mh-darwin
        gen_pkgdata_inc_for_mac_host
    else
        # unknown
        cd "$host_build_dir/data"
        make -f pkgdataMakefile
        cp -r icupkg.inc $pkg_inc_path
    fi
}

gen_icu_data_res() {
    cd $out_put_root_path
    if [ "$host_os"X = "android"X ]; then
        export LD_LIBRARY_PATH=$tool_lib_dir:$i18n_lib_dir:$LD_LIBRARY_PATH
    elif [ "$host_os"X = "ios"X ]; then
        export DYLD_LIBRARY_PATH=$tool_lib_dir:$i18n_lib_dir:$DYLD_LIBRARY_PATH
    else
        export LD_LIBRARY_PATH=$tool_lib_dir:$i18n_lib_dir:$LD_LIBRARY_PATH
    fi

    PYTHONPATH=$icu_source_path/source/python python3 -m icutools.databuilder \
    --src_dir $icu_source_path/source/data \
    --filter_file $icu_data_filter_file \
    --include_uni_core_data \
    --seqmode sequential \
    --mode unix-exec \
    --tool_dir $tool_bin_dir \
    --out_dir $res_out_root_dir/out/build/icudt69l \
    --tmp_dir $res_out_root_dir/out/tmp \
    #--verbose \
}


if ! config_for_host; then
    echo "[ICUData] config host failed."
    exit 1
fi
echo "[ICUData] config host success."

if ! gen_pkgdata_inc; then
    echo "[ICUData] gen pkgdata.inc failed."
    exit 1
fi
echo "[ICUData] gen pkgdata.inc success."

if ! gen_icu_data_res; then
    echo "[ICUData] gen icudata res failed."
    exit 1
fi
echo "[ICUData] gen icudata res success."

# -v Make the output verbose.
# -w Build the data without assembly code.
# -q Quiet mode.
# -c Use the standard ICU copyright.
# -d Specify the destination directory for files
cd $out_put_root_path
PKGDATA="$tool_bin_dir/pkgdata -O $pkg_inc_path -s $res_out_root_dir/out/build/icudt69l -d $res_out_root_dir/out/lib"
$PKGDATA -e icudt69 -T $res_out_root_dir/out/tmp -p icudt69l -m dll -L icudata $res_out_root_dir/out/tmp/icudata.lst
cp -r $res_out_root_dir/out/tmp/icudt69l_dat.S $res_out_root_dir/res/

exit 0