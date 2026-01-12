.text
.global context_switch
.type context_switch, %function

// x0 = old_ctx (ptr), x1 = new_ctx (ptr)
context_switch:
    cbz x0, 1f                 // If old_ctx == 0, skip saving

    // Save callee-saved registers
    stp x19, x20, [x0, #16]
    stp x21, x22, [x0, #32]
    stp x23, x24, [x0, #48]
    stp x25, x26, [x0, #64]
    stp x27, x28, [x0, #80]
    stp x29, x30, [x0, #96]    // x29=FP, x30=LR (Return Address)

    // Save SP
    mov x2, sp
    str x2, [x0, #0]

    // Save LR (x30) as the PC for the next time we resume
    str x30, [x0, #8]

1:
    // Restore SP
    ldr x2, [x1, #0]
    mov sp, x2

    // Restore callee-saved registers
    ldp x19, x20, [x1, #16]
    ldp x21, x22, [x1, #32]
    ldp x23, x24, [x1, #48]
    ldp x25, x26, [x1, #64]
    ldp x27, x28, [x1, #80]
    ldp x29, x30, [x1, #96]

    // Load PC (entry point or return address)
    ldr x3, [x1, #8]

    // Load arguments (only matters for new threads)
    // For resuming threads, this overwrites x0/x1 with whatever was in the struct.
    // Since x0/x1 are caller-saved (volatile), clobbering them here is ABI safe.
    ldr x0, [x1, #112]
    ldr x1, [x1, #120]

    br x3   // Jump

.size context_switch, .-context_switch
