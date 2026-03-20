cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(04_bme)

pico_sdk_init()

add_subdirectory("../libs/protocol" "${PROJECT_BINARY_DIR}/protocol")
add_subdirectory("../libs/bme280-driver" "${PROJECT_BINARY_DIR}/bme280_driver")

add_executable(${PROJECT_NAME}
    main.c
    stdio-task/stdio-task.c
    led-task/led-task.c
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    protocol
    bme280_driver
    hardware_i2c
)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/memmap_rp2040.ld)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})