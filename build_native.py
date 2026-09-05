"""Native C++ compilation script for DHARTI Core Domain Library."""

import os
import shutil
import subprocess
import sys

def build_dharti_core() -> str:
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__)))
    native_dir = os.path.join(project_root, "src", "native")
    include_dir = os.path.join(project_root, "include")

    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        raise RuntimeError("No suitable C++ compiler (g++ or clang++) found in PATH.")

    output_name = "dharti_core.dll" if os.name == "nt" else "libdharti_core.so"
    output_path = os.path.join(native_dir, output_name)

    sources = [
        os.path.join(native_dir, "pci_engine.cpp"),
        os.path.join(native_dir, "contradiction_engine.cpp"),
        os.path.join(native_dir, "sia_inclusion_engine.cpp"),
        os.path.join(native_dir, "dharti_c_api.cpp"),
    ]

    for src in sources:
        if not os.path.exists(src):
            raise FileNotFoundError(f"Source file not found: {src}")

    cmd = [
        compiler,
        "-O3",
        "-shared",
        "-static",
        "-static-libgcc",
        "-static-libstdc++",
        "-std=c++17",
        f"-I{include_dir}",
        f"-I{native_dir}",
        "-o",
        output_path,
    ] + sources

    print(f"Compiling DHARTI C++ Core using {compiler}...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("Compilation failed!", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        sys.exit(result.returncode)

    print(f"Successfully built: {output_path}")
    return output_path

if __name__ == "__main__":
    build_dharti_core()
