#include "common/drivers/fdt.hpp"
#include "common/lib/cstring.hpp"

struct fdt_header {
    std::uint32_t magic;
    std::uint32_t totalsize;
    std::uint32_t off_dt_struct;
    std::uint32_t off_dt_strings;
    std::uint32_t off_mem_rsvmap;
    std::uint32_t version;
    std::uint32_t last_comp_version;
    std::uint32_t boot_cpuid_phys;
    std::uint32_t size_dt_strings;
    std::uint32_t size_dt_struct;
};

constexpr std::uint32_t FDT_BEGIN_NODE = 0x1;
constexpr std::uint32_t FDT_END_NODE   = 0x2;
constexpr std::uint32_t FDT_PROP       = 0x3;
constexpr std::uint32_t FDT_NOP        = 0x4;
constexpr std::uint32_t FDT_END        = 0x9;

// Global State
static void* g_fdt_blob = nullptr;

// The Core Logic
template <typename Func>
static void scan_tree(Func&& callback) {
    if (!g_fdt_blob)
        return;

    const auto* header = static_cast<fdt_header*>(g_fdt_blob);
    if (__builtin_bswap32(header->magic) != 0xd00dfeed)
        return;

    const char* strings = static_cast<char*>(g_fdt_blob) + __builtin_bswap32(header->off_dt_strings);
    auto*       struct_ptr = reinterpret_cast<std::uint32_t*>(
        static_cast<std::uint8_t*>(g_fdt_blob) + __builtin_bswap32(header->off_dt_struct)
    );

    const char* current_node_name = nullptr;
    while (true) {
        switch (std::uint32_t token = __builtin_bswap32(*struct_ptr++)) {
        case FDT_BEGIN_NODE: {
            current_node_name = reinterpret_cast<const char*>(struct_ptr);

            // Align to 4 bytes
            const std::size_t words_to_skip  = (strlen(current_node_name) + 1 + 3) / 4;
            struct_ptr                      += words_to_skip;
            break;
        };
        case FDT_END_NODE: {
            current_node_name = nullptr;
            break;
        };
        case FDT_NOP:
            break;
        case FDT_PROP: {
            std::uint32_t length  = __builtin_bswap32(*struct_ptr++);
            std::uint32_t nameoff = __builtin_bswap32(*struct_ptr++);

            void* prop_data = reinterpret_cast<std::uint8_t*>(struct_ptr);
            struct_ptr += (length + 3) / 4; // Advance the main pointer past the data (aligned)

            if (callback(current_node_name, strings + nameoff, prop_data, length))
                return; // Stop scanning if callback returns true

            break;
        };

        case FDT_END:
            return;
        default:
            break;
        };
    };
};

namespace fdt {
    void initialize(void* dtb_ptr) {
        g_fdt_blob = dtb_ptr;
    };

    // Capture state for the memory search
    static std::uint64_t mem_base  = 0;
    static std::uint64_t mem_size  = 0;
    static bool          mem_found = false;

    // Callback for finding "device_type = memory"
    bool get_memory(std::uint64_t& out_base, std::uint64_t& out_size) {
        mem_found = false;
        scan_tree([](const char* node, const char* prop, void* data,
                     const std::uint32_t length) {
            // Check if this node identifies itself as memory
            if (strcmp(prop, "device_type") == 0) {
                if (strcmp(static_cast<const char*>(data), "memory") == 0) {
                    mem_found = true;
                };
            };

            if (mem_found || (node && strstr(node, "memory") != nullptr)) {
                if (strcmp(prop, "reg") == 0) {
                    if (length < 16)
                        return false; // need at least 2 addr + 2 size cells

                    const auto* cells = static_cast<std::uint32_t*>(data);

                    // Read base address (2 cells)
                    const std::uint64_t base_high = __builtin_bswap32(cells[0]);
                    const std::uint64_t base_low  = __builtin_bswap32(cells[1]);
                    mem_base                      = (base_high << 32) | base_low;

                    // Read size (2 cells)
                    const std::uint64_t size_high = __builtin_bswap32(cells[2]);
                    const std::uint64_t size_low  = __builtin_bswap32(cells[3]);
                    mem_size                      = (size_high << 32) | size_low;

                    return true; // We found RAM
                };
            };

            return false;
        });

        if (mem_size > 0) {
            out_base = mem_base;
            out_size = mem_size;
            return true;
        };

        return false;
    };

    // VirtIO Scanner
    static std::uint32_t target_virtio_id{0};
    static std::uint64_t found_virtio_base{0};

    std::uint64_t find_virtio_device(const std::uint32_t device_id) {
        target_virtio_id  = device_id;
        found_virtio_base = 0;

        scan_tree([](const char* node, const char* prop, void* data,
                     const std::uint32_t length) {
            // We look for the 'reg' property inside any node named 'virtio'
            if (strstr(node, "virtio") && strcmp(prop, "reg") == 0 && length >= 8) {
                const auto* cells = static_cast<std::uint32_t*>(data);

                // Read base address (assuming 2 cells for address)
                const std::uint64_t base_high = __builtin_bswap32(cells[0]);
                const std::uint64_t base_low  = __builtin_bswap32(cells[1]);
                const std::uint64_t addr      = (base_high << 32) | base_low;

                // PROBE THE HARDWARE
                volatile auto* mmio = reinterpret_cast<volatile std::uint32_t*>(addr);

                // Check Magic (0x74726976) and Device ID (Offset 0x008)
                if (mmio[0] == 0x74726976 && mmio[2] == target_virtio_id) {
                    found_virtio_base = addr;
                    return true; // We found it.
                };
            };

            return false;
        });

        return found_virtio_base;
    };
} // namespace fdt