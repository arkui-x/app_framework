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
is_use_ios_simulator="false"

usage()
{
    echo "Options:"
    echo "  -h: Print this message and exit."
    echo "  -o: Arkui root out dir."
    echo "  -n: ICU tool bin root dir."
    echo "  -b: ICU tool bin dir."
    echo "  -l: ICU tool lib dir."
    echo "  -i: ICU i18n lib dir."
    echo "  -t: Target os."
    echo "  -c: Host os."
    echo "  -u: Current cpu."
    echo "  -s: Host platform."
    echo "  -o: Is use ios simulator"
}

while getopts "o:b:l:i:n:t:c:u:s:m:h" arg;
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
        "u")
            current_cpu=${OPTARG}
        ;;
        "s")
            host_platform_dir=${OPTARG}
        ;;
        "m")
            is_use_ios_simulator=${OPTARG}
        ;;
        "h")
            usage
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

gen_pkgdata_inc_for_android() {
    echo "gen_pkgdata_inc_for_android start."

    top_srcdir=$icu_source_path/source
    top_builddir=$host_build_dir
    #linux-x86_64 darwin-x86_64 darwin-arm64
    host_os_arch=$host_platform_dir
    target_os_arch=$current_cpu

    android_sdk_version=29
    android_ndk_root="${ANDROID_HOME}/ndk/21.3.6528147"
    android_toolchain="$android_ndk_root/toolchains/llvm/prebuilt/$host_os_arch"

    TOOLCHAIN=$android_toolchain
    API=$android_sdk_version
    if [ "$target_os_arch"X = "arm64"X ]; then
        target_type="aarch64-linux-android"
        RANLIB=aarch64-linux-android-ranlib
    elif [ "$target_os_arch"X = "arm"X ]; then
        target_type="armv7a-linux-androideabi"
        RANLIB=ranlib
    elif [ "$target_os_arch"X = "x86_64"X ]; then
        target_type="x86_64-linux-android"
        RANLIB=ranlib
    else
        # unknown
        echo "Unsupported this target_os_arch: $target_os_arch"
        return 1
    fi

    GENCCODE_ASSEMBLY="-a gcc"
    SO="so"
    SOBJ="so"
    A="a"
    LIBPREFIX="lib"
    FINAL_SO_TARGET="."
    RPATH_FLAGS=""
    BIR_LDFLAGS="-Wl,-Bsymbolic"
    AR=$TOOLCHAIN/bin/llvm-ar
    ARFLAGS=r
    LDFLAGSICUDT=""
    LD_SONAME=""
    INSTALL=""

    SHAREDLIBCPPFLAGS="-DPIC"
    SHAREDLIBCFLAGS="-fPIC"

        THREADSCPPFLAGS="-D_REENTRANT"
        THREADSCFLAGS=""
        LD_SOOPTIONS="-Wl,-Bsymbolic"

        CC=$TOOLCHAIN/bin/$target_type$API-clang
        CPPFLAGS="-ffunction-sections -fdata-sections $THREADSCPPFLAGS  -DU_HAVE_ELF_H=1 -DU_HAVE_STRTOD_L=1 -DU_HAVE_XLOCALE_H=1 -DU_HAVE_STRING_VIEW=1 "
        DEFS=-DU_ATTRIBUTE_DEPRECATED=
        CFLAGS=-O2 $THREADSCFLAGS
        LDFLAGS=""

    COMPILE="$CC $CPPFLAGS $DEFS $CFLAGS -c"
    SHLIB="$CC $CFLAGS $LDFLAGS -shared $LD_SOOPTIONS"

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

gen_pkgdata_inc_for_ios() {
    top_srcdir=$icu_source_path/source
    top_builddir=$host_build_dir

    DEVELOPER="$(xcode-select --print-path)"
    target_os_arch=$current_cpu
    if [ "$target_os_arch"X = "x64"X ]; then
        target_os_arch="x86_64"
    fi

    if [ "$is_use_ios_simulator"X = "true"X ]; then
        SDKROOT="$(xcodebuild -version -sdk iphonesimulator | grep -E '^Path' | sed 's/Path: //')"
    else
        SDKROOT="$(xcodebuild -version -sdk iphoneos | grep -E '^Path' | sed 's/Path: //')"
    fi

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

        CC="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
        DEFS=-DU_ATTRIBUTE_DEPRECATED=
        CPPFLAGS="-O3 -D__STDC_INT64__ -fno-exceptions -fno-short-wchar -fno-short-enums $THREADSCPPFLAGS  -DU_HAVE_STRTOD_L=1 -DU_HAVE_XLOCALE_H=1 -DU_HAVE_STRING_VIEW=1"
        CFLAGS="-isysroot ${SDKROOT} -I${SDKROOT}/usr/include/ -arch ${target_os_arch} -miphoneos-version-min=10.0 "

    COMPILE="$CC $CPPFLAGS $DEFS $CFLAGS -c"
    SHLIB="$CC $CFLAGS $LDFLAGS -shared $LD_SOOPTIONS"

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
    if [ "$target_os"X = "ios"X ]; then
        # mh-darwin && ios
        gen_pkgdata_inc_for_ios
    elif [ "$target_os"X = "android"X ]; then
        # mh-darwin && android
        gen_pkgdata_inc_for_android
    else
        # unknown
        echo "Unsupported this target_os: $target_os"
        return 1
    fi
}

gen_icu_data_res() {
    cd "$icu_bin_root_out_dir/.."
    if [ "$host_os"X = "linux"X ]; then
        export LD_LIBRARY_PATH=$tool_lib_dir:$i18n_lib_dir:$LD_LIBRARY_PATH
    elif [ "$host_os"X = "mac"X ]; then
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
    --out_dir $res_out_root_dir/out/build/icudt72l \
    --tmp_dir $res_out_root_dir/out/tmp \
    # --verbose
}

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
cd "$icu_bin_root_out_dir/.."

PKGDATA="$tool_bin_dir/pkgdata -s $res_out_root_dir/out/build/icudt72l -T $res_out_root_dir/out/tmp -p icudt72l -d $res_out_root_dir/out"
echo `$PKGDATA $res_out_root_dir/out/tmp/icudata.lst`

exit 0
