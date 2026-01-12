set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Toolchain prefix
set(TOOLCHAIN_PREFIX aarch64-elf)

find_program(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
find_program(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
find_program(CMAKE_AR           ${TOOLCHAIN_PREFIX}-ar)
find_program(CMAKE_LINKER       ${TOOLCHAIN_PREFIX}-ld)
find_program(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}-objcopy)
find_program(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}-objdump)

if (NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "${TOOLCHAIN_PREFIX}-gcc not found in PATH")
endif()

# Bare-metal flags
#set(CMAKE_C_FLAGS "-ffreestanding -fno-threadsafe-statics -fno-exceptions -fno-rtti -nostdlib")
#set(CMAKE_CXX_FLAGS "-ffreestanding -fno-threadsafe-statics -fno-exceptions -fno-rtti -nostdlib")
set(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostdinc -fno-stack-protector")
set(CMAKE_C_FLAGS "-ffreestanding -fno-exceptions -nostdlib -nostdinc -fno-stack-protector")
set(CMAKE_LINKER_FILE "${CMAKE_SOURCE_DIR}/kernel/arch/aarch64/linker.ld")

# Add -mno-outline-atomics to prevent calls to external atomic helpers
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-outline-atomics")