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

echo "[ICUData]++++++++++++++++++Build icu data start.++++++++++++++++++++++"
date +%F' '%H:%M:%S
echo $@

script_path=$(cd $(dirname $0); pwd)
arkui_root_path="$script_path/../../../.."
icu_data_root_path="$script_path/.."
icu_source_path="$arkui_root_path/third_party/icu/icu4c"
icu_build_path="$arkui_root_path/out/arkui-cross_android_arm64/icu_data"
icu_host_install_path="$arkui_root_path/out/arkui-cross_android_arm64/clang_x64/libicudata/android"
icu_android_install_path="$arkui_root_path/out/arkui-cross_android_arm64/libicudata/android"
icu_data_filter_file="$icu_data_root_path/filters/data_filter.json"
icu_data_library_type="static"

add_static_conf_args() {
    icu_configure_args="\
    --enable-strict=no --enable-extras=no --enable-draft=yes \
    --enable-samples=no --enable-tests=no --enable-icuio=no \
    --enable-layoutex=no --with-data-packaging=static --enable-static=yes --enable-shared=no"
}

add_shared_conf_args() {
    icu_configure_args="\
    --enable-strict=no --enable-extras=no --enable-draft=yes \
    --enable-samples=no --enable-tests=no --enable-icuio=no \
    --enable-layoutex=no --with-data-packaging=library --enable-static=no --enable-shared=yes"
}

program_exists() {
    local ret='0'
    command -v $1 >/dev/null 2>&1 || { local ret='1'; }
    if [ "$ret" -ne 0 ]; then
        return 1
    fi
    return 0
}

while getopts "t:h" arg;
do
    case "${arg}" in
        "t")
            icu_data_library_type=${OPTARG}
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

if [ "$icu_data_library_type"X = "shared"X ]; then
    add_shared_conf_args
    icu_build_path="$icu_build_path/shared"
    if ! program_exists patchelf; then
        echo "[ICUData]command patchelf not found, please install with 'sudo apt-get install patchelf'."
        exit 1
    fi
else
    add_static_conf_args
    icu_build_path="$icu_build_path/static"
fi


mkdir -p "$icu_build_path"
if ! icu_build_path=$(realpath "$icu_build_path"); then
    echo "Cannot find real path for icu_build_path '$icu_build_path'."
    exit 1
fi

if ! icu_source_path=$(realpath "$icu_source_path"); then
    echo "Cannot find real path for icu_source_path '$icu_source_path'."
    exit 1
fi

host_os_arch="linux-x86_64"
host_os_build_type="Linux/gcc"
host_build_dir="$icu_build_path/host"

target_os_arch="arm64"
target_type="aarch64-linux-android"

android_sdk_version=29
android_ndk_root="${ANDROID_HOME}/ndk/21.3.6528147"
android_toolchain="$android_ndk_root/toolchains/llvm/prebuilt/$host_os_arch"
android_build_dir="$icu_build_path/android/$target_os_arch"

build_for_host() {
    echo "[ICUData]build host start."
    mkdir -p "$host_build_dir"
    mkdir -p "$icu_host_install_path"

    cd "$host_build_dir"

    export LDFLAGS="-std=gnu++17 -pthread -Wl,--gc-sections"

    if [ -n "$icu_data_filter_file" ]; then
        if ICU_DATA_FILTER_FILE=$(realpath "$icu_data_filter_file"); then
            export ICU_DATA_FILTER_FILE=$ICU_DATA_FILTER_FILE
        fi
    fi

    $icu_source_path/source/runConfigureICU $host_os_build_type $icu_configure_args >/dev/null 2>&1
    
    if ! make -j2 >/dev/null 2>&1; then
        return 1
    fi

    if [ $icu_data_library_type = "shared" ]; then
        echo "[ICUData]install host library."
        cp -r lib/libicudata.so.69.1 $icu_host_install_path/libicudata.so
        patchelf --set-soname libicudata.so $icu_host_install_path/libicudata.so 
    else
        echo "[ICUData]install host library."
        cp -r lib/libicudata.a $icu_host_install_path/libicudata.a
    fi
    return 0
}

build_for_android() {
    echo "[ICUData]build android start."
    mkdir -p "$android_build_dir"
    mkdir -p "$icu_android_install_path"
    cd "$android_build_dir"

    export TOOLCHAIN=$android_toolchain
    export API=$android_sdk_version
    export AR=$TOOLCHAIN/bin/llvm-ar
    export CC=$TOOLCHAIN/bin/$target_type$API-clang
    export AS=$CC
    export CXX=$TOOLCHAIN/bin/$target_type$API-clang++
    export LD=$TOOLCHAIN/bin/ld
    export LDFLAGS="-lc -lstdc++ -Wl,--gc-sections,-rpath-link=$TOOLCHAIN/sysroot/usr/lib/"
    export PATH=$PATH:$TOOLCHAIN/bin

    $icu_source_path/source/configure --with-cross-build="$host_build_dir" $icu_configure_args --host=$target_type >/dev/null 2>&1
        
    if ! make -j2 >/dev/null 2>&1; then
        return 1
    fi

    if [ $icu_data_library_type = "shared" ]; then
        cp -r lib/libicudata.so.69.1 $icu_android_install_path/libicudata.so
        patchelf --set-soname libicudata.so $icu_android_install_path/libicudata.so 
    else
        echo "[ICUData]install android library."
        cp -r lib/libicudata.a $icu_android_install_path/libicudata.a
    fi
    return 0
}

if ! build_for_host; then
    echo "[ICUData]build host failed."
    exit 1
fi
echo "[ICUData]build host success."

if ! build_for_android; then
    echo "[ICUData]build android failed."
    exit 1
fi
echo "[ICUData]android build success."

exit 0
