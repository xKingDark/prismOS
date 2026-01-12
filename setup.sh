#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOOLS_DIR="$ROOT_DIR/tools"
TOOLCHAIN_DIR="$TOOLS_DIR/toolchains"

ARM_GNU_VERSION="13.2.Rel1"
ARM_GNU_NAME="arm-gnu-toolchain-${ARM_GNU_VERSION}-x86_64-aarch64-none-elf"
ARM_GNU_ARCHIVE="${ARM_GNU_NAME}.tar.xz"
ARM_GNU_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_GNU_VERSION}/binrel/${ARM_GNU_ARCHIVE}"

mkdir -p "$TOOLCHAIN_DIR"

# Detect host OS
OS="$(uname -s)"

echo "Setting up prismOS development environment..."

# Install system deps
if [[ "$OS" == "Linux" ]]; then
    if command -v apt >/dev/null 2>&1; then
        echo "Installing system dependencies (apt)..."
        sudo apt update
        sudo apt install -y cmake ninja-build qemu-system-arm wget
    fi
elif [[ "$OS" == "Darwin" ]]; then
    if command -v brew >/dev/null 2>&1; then
        echo "Installing system dependencies (brew)..."
        brew install cmake ninja qemu wget
    else
        echo "Homebrew not found. Please install Homebrew first."
        exit 1
    fi
else
    echo "Unsupported OS: $OS"
    exit 1
fi

# Install toolchain
if [[ ! -d "$TOOLCHAIN_DIR/$ARM_GNU_NAME" ]]; then
    echo "Installing AArch64 bare-metal toolchain..."

    cd "$TOOLCHAIN_DIR"
    wget "$ARM_GNU_URL"
    tar -xf "$ARM_GNU_ARCHIVE"
    rm "$ARM_GNU_ARCHIVE"

    echo "Toolchain installed."
else
    echo "Toolchain already installed."
fi

# Done
echo
echo "Setup complete."
echo "Add the following to your shell config if you want the toolchain globally:"
echo "  export PATH=\"$TOOLCHAIN_DIR/$ARM_GNU_NAME/bin:\$PATH\""
