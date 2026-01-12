cmake_minimum_required(VERSION 3.24)
project(prismOS LANGUAGES C CXX ASM VERSION 1.0.0)

# Sources
file(GLOB_RECURSE KERNEL_COMMON
    "${CMAKE_SOURCE_DIR}/kernel/common/*.cpp"
    "${CMAKE_SOURCE_DIR}/kernel/common/*.hpp"
)
file(GLOB_RECURSE KERNEL_ARCH
    "${CMAKE_SOURCE_DIR}/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/*.[cCsS]"
    "${CMAKE_SOURCE_DIR}/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/*.hpp"
)

add_library(kernel OBJECT ${KERNEL_ARCH} ${KERNEL_COMMON})
target_include_directories(kernel PRIVATE "${CMAKE_SOURCE_DIR}/kernel/")

# Kernel ELF target
add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:kernel>)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}"
    SUFFIX ".elf"
)

# Debugging
set(MAP_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.map)
target_link_options(${PROJECT_NAME} PRIVATE
    -T ${CMAKE_LINKER_FILE}
    -nostdlib
    -static
    -Wl,-Map=${MAP_FILE}
)

# Building the image
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -O binary
        $<TARGET_FILE:${PROJECT_NAME}>
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.bin"
    COMMENT "Generating ${PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.bin"
)