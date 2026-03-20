cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)

project(05_display)

pico_sdk_init()

add_subdirectory("../libs/protocol" "${PROJECT_BINARY_DIR}/protocol")
add_subdirectory("../libs/ili9341-driver" "${PROJECT_BINARY_DIR}/ili9341-driver")

add_executable(${PROJECT_NAME}
    main.c
    stdio-task/stdio-task.c
    led-task/led-task.c
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_spi
    protocol
    ili9341-driver
)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})