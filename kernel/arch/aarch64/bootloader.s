.section .text
.align 4
.global _start
.type _start, %function

_start:
    // Save the DTB pointer (x0) into x19 (callee-saved register) so it survives function calls.
    mov x19, x0

    // Set up stack pointer
    ldr x1, =_stack_top
    mov sp, x1

    // Enable FPU/SIMD (CPACR_EL1)
    //    Bits [20:21] control access to SIMD/FP at EL0/EL1.
    //    (3 << 20) sets both bits to 1 (access allowed).
    mrs x0, cpacr_el1
    orr x0, x0, #(3 << 20)
    msr cpacr_el1, x0
    isb              // Instruction Synchronization Barrier

    // Restore x0 from x19 so _initialize gets the DTB ptr
    mov x0, x19
    bl _initialize

    // Call static constructors
    bl _main

    // Jump to kernel main
    //mov x0, x19 // Pass DTB (x19) to kernel_main as well, just in case
    bl kernel_main
    bl _atexit

1:  wfe
    b 1b

.size _start, . - _start
