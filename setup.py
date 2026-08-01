#!/usr/bin/env python3
import os
import sys
import platform
import subprocess
from pathlib import Path


def run_command(cmd, cwd=None, env=None):
    print(f"--> Executing: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, env=env)
    if result.returncode != 0:
        print(f"Error: Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)


def setup_msvc_env():
    """Finds vswhere and extracts MSVC environment variables on Windows."""
    vswhere_path = Path(os.environ.get("ProgramFiles(x86)", "C:\\Program Files (x86)")) / \
        "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere_path.exists():
        print("Warning: vswhere.exe not found. Assuming environment is already set up.")
        return os.environ.copy()

    cmd = [
        str(vswhere_path),
        "-latest",
        "-products", "*",
        "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
        "-property", "installationPath"
    ]

    vs_path = subprocess.check_output(cmd, text=True).strip()
    if not vs_path:
        print("Warning: Could not find Visual Studio installation.")
        return os.environ.copy()

    vcvars_bat = Path(vs_path) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
    print(f"[Engine Setup] Found MSVC vcvars: {vcvars_bat}")

    # Capture environment after vcvars64.bat runs
    dump_cmd = f'"{vcvars_bat}" && set'
    output = subprocess.check_output(dump_cmd, shell=True, text=True)

    env = {}
    for line in output.splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            env[key] = value
    return env


def main():
    root_dir = Path(__file__).parent.resolve()
    print(f"[Engine Setup] Setting up graphics engine in: {root_dir}")

    print("[Engine Setup] Syncing Git Submodules...")
    run_command(["git", "submodule", "update",
                "--init", "--recursive"], cwd=root_dir)

    current_os = platform.system()
    custom_env = os.environ.copy()
    cmake_args = ["cmake", "-B", "build", "-G",
                  "Ninja", "-DCMAKE_BUILD_TYPE=Debug"]

    if current_os == "Windows":
        custom_env = setup_msvc_env()
        cmake_args.extend(["-DCMAKE_C_COMPILER=cl", "-DCMAKE_CXX_COMPILER=cl"])
    elif current_os == "Linux":
        print("[Engine Setup] Configuring for Linux Environment...")
    elif current_os == "Darwin":
        print("[Engine Setup] Configuring for macOS Environment...")

    # 3. Configure CMake
    print("[Engine Setup] Running CMake Configuration...")
    run_command(cmake_args, cwd=root_dir, env=custom_env)

    print("\n[Engine Setup] Setup Complete! You can now run:")
    print("  ninja -C build")


if __name__ == "__main__":
    main()
