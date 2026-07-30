#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

# Description
#
# This script is invoked by the build system and does not need to be executed directly by the developer.
# It verifies if the node_modules folder exists in the target output tree.
# If not, npm install is executed. TypeScript emits WindowEnv js files directly
# to the target output directory.

import os
import sys
import time
import shutil
import subprocess


def copy_if_different(src_file, dst_file):
    if os.path.exists(dst_file):
        with open(src_file, "rb") as src_stream, open(dst_file, "rb") as dst_stream:
            if src_stream.read() == dst_stream.read():
                return False
    shutil.copy2(src_file, dst_file)
    return True


def sync_npm_manifest(project_path, install_dir):
    os.makedirs(install_dir, exist_ok=True)

    changed = False
    for filename in ["package.json", "package-lock.json"]:
        src_file = os.path.join(project_path, filename)
        dst_file = os.path.join(install_dir, filename)
        if not os.path.exists(src_file):
            print(f"windowenv: {src_file} not exist")
            sys.exit(1)
        changed = copy_if_different(src_file, dst_file) or changed
    return changed


def is_tsc_available(node_modules_path):
    tsc_path = os.path.join(node_modules_path, "typescript", "lib", "tsc.js")
    if os.path.exists(tsc_path):
        print(f"windowenv: tsc found at {tsc_path}")
        return True
    print(f"windowenv: tsc not found at {tsc_path}")
    return False


def run_npm_install(install_dir):
    secondary_npm_registry = "https://registry.npmjs.org/"
    try:
        subprocess.check_call(["npm", "install"], cwd=install_dir)
    except subprocess.CalledProcessError as e:
        print(f"windowenv: npm install failed with exit code {e.returncode}. Retry...")
        try:
            subprocess.check_call([
                "npm", "install",
                "--registry", secondary_npm_registry,
                "--loglevel=verbose"
            ], cwd=install_dir)
        except subprocess.CalledProcessError as e2:
            sys.exit(e2.returncode)


def run_tsc(project_path, node_modules_path, js_output_path):
    tsc_path = os.path.join(node_modules_path, "typescript", "lib", "tsc.js")
    cmd = [
        "node",
        tsc_path,
        "--project",
        os.path.join(project_path, "tsconfig.json"),
        "--outDir",
        js_output_path,
    ]
    print(f"windowenv: Running {' '.join(cmd)}")
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        print(f"windowenv: tsc failed with exit code {e.returncode}.")
        sys.exit(e.returncode)


def check_js_outputs(js_output_path):
    expected_files = [
        "WindowSizeEnv.js",
        "WindowAvoidAreaEnv.js",
    ]
    for filename in expected_files:
        output_file = os.path.join(js_output_path, filename)
        if not os.path.exists(output_file):
            print(f"windowenv: Built file not found at {output_file}")
            sys.exit(1)


def main(argv):
    if len(argv) < 4:
        print("windowenv: python build.py <path_to_project> <path_to_node_modules> <js_output_path>")
        sys.exit(1)

    project_path = os.path.abspath(argv[1])
    node_modules_path = os.path.abspath(argv[2])
    js_output_path = os.path.abspath(argv[3])
    install_dir = os.path.dirname(node_modules_path)

    print(f"windowenv: Changing directory to {project_path}. Out dir = {js_output_path}")
    if not os.path.exists(project_path):
        print(f"windowenv: {project_path} not exist")
        sys.exit(1)

    manifest_changed = sync_npm_manifest(project_path, install_dir)

    if manifest_changed:
        print("windowenv: npm manifest changed, running npm install")
        run_npm_install(install_dir)
    elif not os.path.exists(node_modules_path):
        print(f"windowenv: node_modules directory not found at {node_modules_path}, running npm install")
        run_npm_install(install_dir)
    elif not is_tsc_available(node_modules_path):
        print("windowenv: TypeScript compiler missing, running npm install")
        run_npm_install(install_dir)
    else:
        print(f"windowenv: node_modules directory exists at {node_modules_path}")

    if not is_tsc_available(node_modules_path):
        print("windowenv: TypeScript compiler is unavailable after npm install.")
        sys.exit(1)

    if os.path.exists(js_output_path):
        shutil.rmtree(js_output_path)
    os.makedirs(js_output_path)

    run_tsc(project_path, node_modules_path, js_output_path)
    check_js_outputs(js_output_path)

if __name__ == '__main__':
    start_time = time.time()
    main(sys.argv)
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"windowenv: build time: {elapsed_time:.2f} seconds")
