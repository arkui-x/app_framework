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

echo "[ICUData] ++++++++++++++++++Build icu data start.++++++++++++++++++++++"
date +%F' '%H:%M:%S
echo $@

script_path=$(cd $(dirname $0); pwd)
arkui_root_path="$script_path/../../../.."
icu_data_root_path="$script_path/.."
icu_source_path="$arkui_root_path/third_party/icu/icu4c"
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

while getopts "t:o:h" arg;
do
    case "${arg}" in
        "t")
            icu_data_library_type=${OPTARG}
        ;;
        "o")
            out_put_root_path=${OPTARG}
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

icu_build_path="$arkui_root_path/$out_put_root_path/icu_data"
#icu_host_install_path="$arkui_root_path/$out_put_root_path/libicudata/ios"
icu_ios_install_path="$arkui_root_path/$out_put_root_path/libicudata/ios"

if [ "$icu_data_library_type"X = "shared"X ]; then
    add_shared_conf_args
    icu_build_path="$icu_build_path/shared"
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
host_os_build_type="MacOSX"
host_build_dir="$icu_build_path/host"

target_os_arch="arm64"
target_type="aarch64-apple-darwin"

DEVELOPER="$(xcode-select --print-path)"
SDKROOT="$(xcodebuild -version -sdk iphoneos | grep -E '^Path' | sed 's/Path: //')"

ios_build_dir="$icu_build_path/ios/$target_os_arch"
icu_ios_source_patch_path="$ios_build_dir/patch"

build_for_host() {
    echo "[ICUData] build host start."
    mkdir -p -v "$host_build_dir"
    #mkdir -p "$icu_host_install_path"

    cd "$host_build_dir"

    if [ -n "$icu_data_filter_file" ]; then
        if ICU_DATA_FILTER_FILE=$(realpath "$icu_data_filter_file"); then
            export ICU_DATA_FILTER_FILE=$ICU_DATA_FILTER_FILE
        fi
    fi

    export CFLAGS="-O3 -D__STDC_INT64__ -fno-exceptions -fno-short-wchar -fno-short-enums"
    export CXXFLAGS="${CFLAGS} -std=c++11"
    export CPPFLAGS=${CFLAGS}
    $icu_source_path/source/runConfigureICU $host_os_build_type $icu_configure_args >/dev/null 2>&1
    
    if ! make -j2 >/dev/null 2>&1; then
        return 1
    fi
    return 0
}

build_for_ios() {
    echo "[ICUData] build ios start."
    mkdir -p -v "$icu_ios_source_patch_path"

    cp -R ${icu_source_path}/* "$icu_ios_source_patch_path/"

    echo "===== Patching icu/source/tools/pkgdata/pkgdata.cpp for iOS ====="
    cp "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata" "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.cpp"
    cp "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.cpp" "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata"

    rm -f "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.tmp"
    mktemp "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.tmp"

    sed "s/int result = system(cmd);/ \\
    #if defined(IOS_SYSTEM_PATCH) \\
    pid_t pid; \\
    char * argv[2]; argv[0] = cmd; argv[1] = NULL; \\
    posix_spawn(\&pid, argv[0], NULL, NULL, argv, environ); \\
    waitpid(pid, NULL, 0); \\
    int result = 0; \\
    #else \\
    int result = system(cmd); \\
    #endif \\
    /g" "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.cpp" > "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.tmp"

    sed "/#include <stdlib.h>/a \\
    #if defined(IOS_SYSTEM_PATCH) \\
    #include <spawn.h> \\
    extern char **environ; \\
    #endif \\
    " "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.tmp" > "${icu_ios_source_patch_path}/source/tools/pkgdata/pkgdata.cpp"

    mkdir -p "$ios_build_dir"
    mkdir -p "$icu_ios_install_path"

    cd "$ios_build_dir"
    
    unset CXX
    unset CC
    unset CFLAGS
    unset CXXFLAGS
    unset LDFLAGS

    ICU_FLAGS="-I${icu_ios_source_patch_path}/source/common/ -I${icu_ios_source_patch_path}/source/tools/tzcode/ "

    export ADDITION_FLAG="-DIOS_SYSTEM_PATCH"
    export CXX="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
    export CC="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
    export CFLAGS="-isysroot ${SDKROOT} -I${SDKROOT}/usr/include/ -I./include/ -arch ${target_os_arch} -miphoneos-version-min=9.0 ${ICU_FLAGS} ${CFLAGS} ${ADDITION_FLAG}"
    export CXXFLAGS="${CXXFLAGS} -stdlib=libc++ -isysroot ${SDKROOT} -I${SDKROOT}/usr/include/ -I./include/ -arch ${target_os_arch} -miphoneos-version-min=9.0 ${ICU_FLAGS} ${ADDITION_FLAG}"
    export LDFLAGS="-stdlib=libc++ -L${SDKROOT}/usr/lib/ -isysroot ${SDKROOT} -Wl,-dead_strip -miphoneos-version-min=9.0 -lstdc++ ${ADDITION_FLAG}"

    ${icu_ios_source_patch_path}/source/configure --with-cross-build=${host_build_dir} --host=${target_type} ${icu_configure_args} >/dev/null 2>&1
        
    if ! make -j2 >/dev/null 2>&1; then
        return 1
    fi

    if [ $icu_data_library_type = "shared" ]; then
        cp -r lib/libicudata.69.1.dylib $icu_ios_install_path/libicudata.dylib
    else
        echo "cp libicudata.a"
        cp -r lib/libicudata.a $icu_ios_install_path/libicudata.a
    fi
    return 0
}

if ! build_for_host; then
     echo "[ICUData] build host failed."
     exit 1
fi
echo "[ICUData] build host success."

if ! build_for_ios; then
    echo "[ICUData] build ios failed."
    exit 1
fi
echo "[ICUData] build ios success."

exit 0
