<div align="center">

# prismOS
A freestanding AArch64 (ARMv8) operating system kernel made for fun.
</div>

## Requirements
### Build Tools (All Platforms)
- [CMake](https://cmake.org/download) ≥ 3.24
- Ninja
- AArch64 ELF cross toolchain
  - `aarch64-elf-gcc`
  - `aarch64-elf-g++`
  - `aarch64-elf-ld`
  - `aarch64-elf-ar`
  - `aarch64-elf-objcopy`
  - `aarch64-elf-objdump`
> [!WARNING]
> This project targets bare-metal AArch64.
> Do not use `aarch64-linux-gnu-gcc`.

---
### Runtime
QEMU with `qemu-system-aarch64`

## Platform Setup
### macOS (Intel & Apple Silicon)
```shell
brew install cmake ninja qemu aarch64-elf-gcc
```
### Linux (Debian / Ubuntu)
```shell
sudo apt update
sudo apt install \
    cmake ninja-build qemu-system-arm \
    gcc-aarch64-elf binutils-aarch64-elf
```
### Windows
Recommended options:

Option A: MSYS2 (MinGW64 shell)
```shell
pacman -S \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-qemu \
    mingw-w64-x86_64-aarch64-elf-gcc
```
Option B: WSL2 — Use Ubuntu and follow the Linux instructions.

## Quick Start
This project uses CMake Presets for consistent builds.
### Debug Build
```shell
cmake --preset aarch64-debug
cmake --build --preset aarch64-debug
```
### Release Build
```shell
cmake --preset aarch64-release
cmake --build --preset aarch64-release
```
### Build Outputs
Each build produces:
```
prismOS.aarch64.elf   # Linked kernel (symbols included)
prismOS.aarch64.bin   # Raw bootable image
prismOS.aarch64.map   # Linker map file
```
Located in:
```
build/aarch64-debug/
build/aarch64-release/
```

## Running prismOS in QEMU
### macOS / Linux
```shell
./run-aarch64.sh
```
Defaults to Debug.
```shell
./run-aarch64.sh release
```
Supported Modes:
```shell
./run-aarch64.sh debug
./run-aarch64.sh release
BUILD_TYPE=release ./run-aarch64.sh
```

---
### Design Notes
- Fully freestanding (-nostdlib)
- No host libc or runtime
- Toolchain enforced via CMake toolchain file

## License
This project is open-source under the **MIT License**. Feel free to modify and contribute!

## Contributing
```shell
# Fork the repository
git clone https://github.com/xKingDark/prismOS.git

# Create a new branch
git checkout -b feature-branch

# Commit your changes
git commit -m "Added new feature"

# Push to GitHub
git push origin feature-branch

# Submit a Pull Request
```