#ifndef FDT_HPP
#define FDT_HPP
#include <common/std/stdint.hpp>

namespace fdt {
    // Must be called in kernel_main before using other functions
    void initialize(void* dtb_ptr);

    // Returns true if memory node found, filling base and size
    bool get_memory(std::uint64_t& out_base, std::uint64_t& out_size);

    // Find a VirtIO MMIO device with a specific Device ID
    // device_id: 1 = Net, 2 = Block, etc.
    std::uint64_t find_virtio_device(std::uint32_t device_id);
}; // namespace fdt

#endif // FDT_HPP