# prismOS Architecture
This document describes the high-level architecture of **prismOS**.
It is intended to provide an overview of system structure and design decisions,
not a detailed implementation guide.

---

## Target Architecture
- **Architecture:** AArch64 (ARMv8)
- **Execution Level:** EL1 (kernel mode)
- **Environment:** Bare-metal
- **Boot Method:** Direct kernel loading via QEMU
- **SMP:** Not supported (single-core only)

prismOS does not currently target UEFI or a multiboot specification.

---

## Kernel Layout
The kernel is divided into two primary layers:
```
kernel/
├── common/ # Architecture-independent code
└── arch/aarch64/ # AArch64-specific code
```

### `kernel/common`
Contains logic intended to be portable across architectures, including:
- Core utilities
- Kernel abstractions
- Shared subsystems

### `kernel/arch/aarch64`
Contains architecture-specific code, including:
- Boot and entry code
- Exception vectors
- Low-level CPU setup
- Platform-specific initialization

---

## Execution Model
prismOS runs entirely in kernel mode and supports **cooperative kernel threads**.
- No userspace (EL0)
- No processes
- No preemption
- No timer-driven scheduling

All context switches occur explicitly via `yield()` or thread termination.

---

## Threading Model
Threads are kernel-managed execution contexts with independent stacks.

Each thread consists of:
- A dynamically allocated stack
- A saved AArch64 CPU context
- A lifecycle state

### Thread Context
The saved context includes:
- Stack pointer (SP)
- Program counter (PC)
- Callee-saved registers (`x19–x30`)
- Initial argument registers (`x0`, `x1`) for thread entry

Context switching is implemented in architecture-specific assembly and follows
the AArch64 ABI.

---

## Scheduler
The scheduler implements a **round-robin** policy using a fixed-size ring buffer.
- Maximum number of runnable threads is bounded
- Threads explicitly re-enter the run queue via `yield()`
- DEAD threads are skipped and never rescheduled
- When no runnable threads remain, the system enters an idle state (`wfe`)

There is no notion of priority or preemption.

---

## Memory Management
prismOS uses a single global heap with no virtual memory.
- MMU is disabled
- Memory is identity-mapped
- Heap boundaries are defined at boot time

### Allocator Design
The allocator combines:
- A **bump pointer** for new allocations
- A **free list** for reclaimed blocks

Properties:
- 16-byte alignment
- No block coalescing
- No per-thread heaps
- No allocator locking (single-core assumption)

This allocator is sufficient for early kernel development and debugging.

---

## Toolchain and Build
- The kernel is built using an **AArch64 ELF cross toolchain**
- Builds are fully freestanding (`-nostdlib`)
- Output artifacts:
    - ELF (symbols)
    - Raw binary (bootable image)
    - Linker map (memory layout)

---

## Non-Goals (For Now)
The following are explicitly not implemented:
- Userspace support
- Preemptive multitasking
- SMP / multicore support
- Virtual memory or paging
- Filesystems
- Full networking stack
- Security or isolation guarantees

These may be explored in future iterations.

---

## Design Philosophy
prismOS prioritizes:
- Simplicity over completeness
- Explicit control over abstraction
- Clear separation between architecture-specific and generic code

The architecture is expected to evolve as new subsystems are introduced.