set(TARGET_FLOAT_ABI soft)
set(TARGET_FPU none)
set(TARGET_CPU cortex-m0plus)
set(TARGET_ARCH v6-m)
set(TARGET_TRIPLE thumbv6-m-none-eabi)
set(TARGET_ARM_INSTRUCTION_MODE thumb)
set(TARGET_ENDIAN little-endian)

svd_convert(core_peripherals SVD_FILE ${CMAKE_CURRENT_LIST_DIR}/../core.svd OUTPUT_DIRECTORY core_peripherals)
