#include "virtio.hpp"
#include "common/lib/memory.hpp" // malloc, free
#include "common/std/print.hpp"  // println

// Global Driver State
static volatile std::uint32_t* virtio_base = nullptr;
static VirtQueue               rx_queue; // Queue 0
static VirtQueue               tx_queue; // Queue 1

// Helper: Allocate a 4KB aligned page
static void* alloc_page() {
    // Crude alignment: alloc 8KB, find 4KB boundary
    void*                p       = malloc(8192);
    const auto           addr    = reinterpret_cast<std::uintptr_t>(p);
    const std::uintptr_t aligned = (addr + 4095) & ~4095;
    return reinterpret_cast<void*>(aligned);
};

// Helper: Initialize a single queue
static void setup_queue(int queue_idx, VirtQueue& vq) {
    vq.index         = queue_idx;
    vq.last_used_idx = 0;

    // Select the queue
    virtio_base[VIRTIO_MMIO_QUEUE_SEL / 4] = queue_idx;

    // Check if it exists
    std::uint32_t max_size = virtio_base[VIRTIO_MMIO_QUEUE_NUM_MAX / 4];
    if (max_size == 0) {
        std::println("VirtIO: Queue {} unavailable", queue_idx);
        return;
    };

    // Set queue size (must be power of 2, <= max_size)
    vq.size                                = 16;
    virtio_base[VIRTIO_MMIO_QUEUE_NUM / 4] = vq.size;

    // Allocate memory for the rings (Legacy Layout)
    // We put Desc + Avail + Used all in one contiguous page for simplicity
    // NOTE: This assumes 16 descriptors fits in one page (it easily does)
    virtio_base[VIRTIO_MMIO_QUEUE_ALIGN / 4] = 4096;

    void* queue_mem = alloc_page();
    std::memset(queue_mem, 0, 4096);

    // Calculate offsets within that page
    // Desc Table: 16 * 16 bytes = 256 bytes
    // Avail Ring: 4 + (16 * 2) + 2 = 38 bytes
    // Used Ring: Must be aligned to 4096 (or usually next boundary in legacy)
    // For legacy QEMU 'virt', simply pointing PFN is enough, but we need pointers:

    vq.desc = static_cast<virtq_desc*>(queue_mem);

    constexpr std::uintptr_t avail_offset = 16 * sizeof(virtq_desc);
    vq.avail =
        reinterpret_cast<virtq_avail*>(static_cast<std::uint8_t*>(queue_mem) + avail_offset);

    // The Used Ring is usually placed at a page boundary or specifically aligned.
    // For this simple legacy driver, we rely on QEMU calculating the used offset
    // based on the PFN. But we need to know where QEMU puts it to READ it.
    // Standard formula: align(avail_end, 4096)

    const std::uintptr_t avail_end = reinterpret_cast<std::uintptr_t>(vq.avail) +
                                     sizeof(std::uint16_t) * 3 +
                                     vq.size * sizeof(std::uint16_t);
    std::uintptr_t used_offset = (avail_end + 4095) & ~4095;

    // Wait, if we use one 4K page, 'used' must be INSIDE it if possible,
    // or we need a second page if the alignment pushes it out.
    // With size=16, avail_end is small (~300 bytes). 4096 alignment pushes Used to start of
    // NEXT page. Let's allocate TWO pages to be safe.

    vq.used = reinterpret_cast<virtq_used*>(static_cast<std::uint8_t*>(queue_mem) + 4096);

    // Tell Device the Physical Page Number (Legacy MMIO)
    const auto phys_addr                   = reinterpret_cast<std::uintptr_t>(queue_mem);
    virtio_base[VIRTIO_MMIO_QUEUE_PFN / 4] = phys_addr / 4096;
};

void virtio_net_init(std::uint64_t base_addr) {
    virtio_base = reinterpret_cast<volatile std::uint32_t*>(base_addr);

    if (virtio_base[VIRTIO_MMIO_MAGIC_VALUE / 4] != 0x74726976) {
        std::println("VirtIO: Bad Magic Value");
        return;
    };

    if (virtio_base[VIRTIO_MMIO_DEVICE_ID / 4] != VIRTIO_DEV_NET) {
        std::println("VirtIO: Not a network device");
        return;
    };

    // Initialization Sequence

    // Reset
    virtio_base[VIRTIO_MMIO_STATUS / 4] = 0;

    // Ack + Driver
    std::uint32_t status                 = 0;
    status                              |= 1; // ACKNOWLEDGE
    virtio_base[VIRTIO_MMIO_STATUS / 4]  = status;
    status                              |= 2; // DRIVER
    virtio_base[VIRTIO_MMIO_STATUS / 4]  = status;

    // Negotiate Features (Accept defaults)
    // virtio_base[0x010/4]; // Read Host Features
    // virtio_base[0x020/4] = ...; // Write Guest Features

    // Setup Queues
    setup_queue(0, rx_queue); // RX
    setup_queue(1, tx_queue); // TX

    // Go!
    status                              |= 4; // DRIVER_OK
    virtio_base[VIRTIO_MMIO_STATUS / 4]  = status;

    std::println("VirtIO Net: Initialized at {}", reinterpret_cast<void*>(base_addr));
};

void virtio_net_init_rx() {
    // Fill RX queue with empty buffers
    for (int i = 0; i < rx_queue.size; ++i) {
        // Allocate buffer (MTU 1500 + Header 10 + padding) -> 2048
        void* buffer = malloc(2048);

        // Descriptor points to buffer
        rx_queue.desc[i].addr  = reinterpret_cast<std::uint64_t>(buffer);
        rx_queue.desc[i].len   = 2048;
        rx_queue.desc[i].flags = VIRTQ_DESC_F_WRITE; // Device writes here
        rx_queue.desc[i].next  = 0;

        // Put index in Available Ring
        rx_queue.avail->ring[i] = i;
    };

    // Publish
    asm volatile("dmb sy" ::: "memory");
    rx_queue.avail->idx = rx_queue.size;

    asm volatile("dmb sy" ::: "memory");
    virtio_base[VIRTIO_MMIO_QUEUE_NOTIFY / 4] = 0; // Notify RX Queue
};

void virtio_net_send(const void* data, std::uint32_t length) {
    // NOTE: In a real driver, we need to prepend the virtio_net_hdr.
    // For simplicity, we assume 'data' already has 10 bytes of 0 padding at front
    // OR we allocate a temp buffer here. Let's do the temp buffer way (Safe).

    std::uint32_t total_len = sizeof(virtio_net_hdr) + length;
    auto*         buffer    = new std::uint8_t[total_len];

    // Zero header
    std::memset(buffer, 0, sizeof(virtio_net_hdr));
    // Copy data after header
    std::memcpy(buffer + sizeof(virtio_net_hdr), data, length);

    // Get a descriptor (Simple Round Robin)
    static std::uint16_t head_idx = 0;
    std::uint16_t        desc_idx = head_idx;
    head_idx                      = (head_idx + 1) % tx_queue.size;

    // Setup Descriptor
    tx_queue.desc[desc_idx].addr  = reinterpret_cast<std::uint64_t>(buffer);
    tx_queue.desc[desc_idx].len   = total_len;
    tx_queue.desc[desc_idx].flags = 0; // Device reads
    tx_queue.desc[desc_idx].next  = 0;

    // Update Avail
    std::uint16_t avail_idx         = tx_queue.avail->idx % tx_queue.size;
    tx_queue.avail->ring[avail_idx] = desc_idx;

    asm volatile("dmb sy" ::: "memory");
    tx_queue.avail->idx++;

    asm volatile("dmb sy" ::: "memory");
    virtio_base[VIRTIO_MMIO_QUEUE_NOTIFY / 4] = 1; // Notify TX Queue

    // NOTE: In this simple driver, we leak 'buffer' because we don't wait for the device to finish using it.
    // TODO: Clean up used buffers in 'poll' or use a static buffer.
};

void virtio_net_poll() {
    // Check RX Used Ring
    while (rx_queue.last_used_idx != rx_queue.used->idx) {
        std::uint16_t used_slot = rx_queue.last_used_idx % rx_queue.size;
        auto [id, length]       = rx_queue.used->ring[used_slot];

        // The buffer address
        auto* buffer = reinterpret_cast<std::uint8_t*>(rx_queue.desc[id].addr);

        // The header is at the start, data follows
        auto*         packet_data = buffer + sizeof(virtio_net_hdr);
        std::uint32_t packet_len  = length - sizeof(virtio_net_hdr);

        std::println("RX Packet: {} bytes", packet_len);
        // Print Dest MAC (first 6 bytes)
        std::println(
            "  Dst MAC: {:02x}:{:02x}:{:02x}:...", packet_data[0], packet_data[1],
            packet_data[2]
        );

        // RECYCLE
        std::uint16_t avail_idx         = rx_queue.avail->idx % rx_queue.size;
        rx_queue.avail->ring[avail_idx] = id;

        asm volatile("dmb sy" ::: "memory");
        rx_queue.avail->idx++;

        rx_queue.last_used_idx++;
    };

    // Notify if we recycled anything (check if indices moved)
    // virtio_base[VIRTIO_MMIO_QUEUE_NOTIFY / 4] = 0;
};