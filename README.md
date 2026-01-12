<div align="center">

# prismOS
A freestanding AArch64 (ARMv8) operating system kernel made for fun.
</div>

## Requirements
### Build Tools (All Platforms)
- [CMake](https://cmake.org/download) ≥ 3.24
- Ninja
- QEMU (`qemu-system-aarch64`)

> [!NOTE]
> The AArch64 bare-metal toolchain is automatically downloaded and configured by `setup.sh`.
> No manual toolchain installation is required.

## Platform Setup
### Linux (Debian / Ubuntu)
```shell
sudo apt update
```

Then run
```shell
./setup.sh
```

### macOS (Intel & Apple Silicon)
Install [Homebrew](https://brew.sh), then run
```shell
./setup.sh
```

### Windows
Recommended options:
- [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) (Ubuntu)

After cloning the repository, follow the Linux instructions.

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