#pragma once
#include <common/std/stdint.hpp>

// VirtIO MMIO Register Offsets
constexpr std::uint32_t VIRTIO_MMIO_MAGIC_VALUE   = 0x000;
constexpr std::uint32_t VIRTIO_MMIO_VERSION       = 0x004;
constexpr std::uint32_t VIRTIO_MMIO_DEVICE_ID     = 0x008; // 1=Net
constexpr std::uint32_t VIRTIO_MMIO_VENDOR_ID     = 0x00C;
constexpr std::uint32_t VIRTIO_MMIO_STATUS        = 0x070;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_SEL     = 0x030;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_NUM_MAX = 0x034;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_NUM     = 0x038;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_ALIGN   = 0x03C;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_PFN     = 0x040;
constexpr std::uint32_t VIRTIO_MMIO_QUEUE_NOTIFY  = 0x050;

// Device Constants
constexpr std::uint32_t VIRTIO_DEV_NET     = 1;
constexpr std::uint16_t VIRTQ_DESC_F_NEXT  = 1;
constexpr std::uint16_t VIRTQ_DESC_F_WRITE = 2; // Device writes to this buffer

// VirtIO Ring Structures (Hardware Layout)

// Descriptor Table Entry
struct virtq_desc {
    std::uint64_t addr;
    std::uint32_t len;
    std::uint16_t flags;
    std::uint16_t next;
} __attribute__((packed));

// Available Ring (Driver -> Device)
struct virtq_avail {
    std::uint16_t flags;
    std::uint16_t idx;
    std::uint16_t ring[]; // Flexible array member
} __attribute__((packed));

// Used Ring Element
struct virtq_used_elem {
    std::uint32_t id;
    std::uint32_t length;
} __attribute__((packed));

// Used Ring (Device -> Driver)
struct virtq_used {
    std::uint16_t   flags;
    std::uint16_t   idx;
    virtq_used_elem ring[]; // Flexible array member
} __attribute__((packed));

// Driver State
// This tracks the memory locations of the rings for one queue
struct VirtQueue {
    std::uint32_t index;         // 0 = RX, 1 = TX
    std::uint16_t size;          // Number of descriptors (e.g., 16)
    std::uint16_t last_used_idx; // Driver's tracking index for Used Ring

    // Pointers to the actual ring memory
    virtq_desc*  desc;
    virtq_avail* avail;
    virtq_used*  used;
};

// VirtIO Net Header
// Every packet in VirtIO-Net is preceded by this header.
// We must allocate space for it, but we can mostly ignore it for now.
struct virtio_net_hdr {
    std::uint8_t  flags;
    std::uint8_t  gso_type;
    std::uint16_t hdr_len;
    std::uint16_t gso_size;
    std::uint16_t csum_start;
    std::uint16_t csum_offset;
    // std::uint16_t num_buffers; // Only if VIRTIO_NET_F_MRG_RXBUF is negotiated
} __attribute__((packed));

// Call this once with the MMIO base address found in FDT
void virtio_net_init(std::uint64_t base_addr);

// Call this to initialize the receive buffers
void virtio_net_init_rx();

// Call this to send raw data (Ethernet frame)
void virtio_net_send(const void* data, std::uint32_t length);

// Call this in your main loop to check for incoming packets
void virtio_net_poll();