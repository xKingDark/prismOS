#!/usr/bin/env bash
set -e

BUILD_TYPE="${1:-${BUILD_TYPE:-debug}}"
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

case "$BUILD_TYPE" in
debug)
    BUILD_DIR="build/aarch64-debug"
    ;;
release)
    BUILD_DIR="build/aarch64-release"
    ;;
*)
    echo "Usage: $0 [debug|release]"
    exit 1
    ;;
esac

KERNEL="${BUILD_DIR}/prismOS.aarch64.bin"

if [[ ! -f "$KERNEL" ]]; then
    echo "Error: kernel not found: $KERNEL"
    echo "Did you build it with the correct preset?"
    exit 1
fi

qemu-system-aarch64 \
    -machine virt \
    -cpu cortex-a53 \
    -nographic \
    -serial mon:stdio \
    -m 4G \
    -kernel "$KERNEL" \
    -netdev user,id=net0 \
    -device virtio-net-device,netdev=net0
