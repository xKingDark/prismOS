set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Toolchain prefix
set(TOOLCHAIN_PREFIXES
    aarch64-none-elf
    aarch64-elf
)

foreach(prefix IN LISTS TOOLCHAIN_PREFIXES)
    if(NOT CMAKE_C_COMPILER)
        find_program(CMAKE_C_COMPILER   ${prefix}-gcc)
        find_program(CMAKE_CXX_COMPILER ${prefix}-g++)
        find_program(CMAKE_ASM_COMPILER ${prefix}-gcc)
        find_program(CMAKE_AR           ${prefix}-ar)
        find_program(CMAKE_LINKER       ${prefix}-ld)
        find_program(CMAKE_OBJCOPY      ${prefix}-objcopy)
        find_program(CMAKE_OBJDUMP      ${prefix}-objdump)
    endif()
endforeach()

if (NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "No AArch64 bare-metal toolchain found (aarch64-none-elf or aarch64-elf)")
endif()

# Bare-metal flags
#set(CMAKE_C_FLAGS "-ffreestanding -fno-threadsafe-statics -fno-exceptions -fno-rtti -nostdlib")
#set(CMAKE_CXX_FLAGS "-ffreestanding -fno-threadsafe-statics -fno-exceptions -fno-rtti -nostdlib")
set(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -fno-stack-protector")
set(CMAKE_C_FLAGS "-ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-stack-protector")
set(CMAKE_LINKER_FILE "${CMAKE_SOURCE_DIR}/kernel/arch/aarch64/linker.ld")

# Add -mno-outline-atomics to prevent calls to external atomic helpers
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-outline-atomics")