cmake_minimum_required(VERSION 3.13)

# Set default board to Pico 2 W if not specified
if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico2_w)
endif()

if(NOT DEFINED PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    else()
        message(FATAL_ERROR "PICO_SDK_PATH is not set. Export it or pass -DPICO_SDK_PATH=/path/to/pico-sdk.")
    endif()
endif()

set(ProjectName rp2040-hostap-i2c)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel)

include(pico_sdk_import.cmake)

project(${ProjectName} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Check for FreeRTOS kernel
if(NOT EXISTS "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake")
    message(FATAL_ERROR "FreeRTOS kernel not found under ${FREERTOS_KERNEL_PATH}. "
                        "Run 'git submodule update --init --recursive' from the repo root.")
endif()

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

add_executable(${ProjectName}
    src/main.c
    # Drivers
    src/drivers/ina238.c
    src/drivers/bme280.c
    # Tasks
    src/tasks/sensor_task.c
    src/tasks/wifi_task.c
    src/tasks/webserver_task.c
    # Web content
    src/web/fsdata.c
)

# Enable USB serial output for debugging
pico_enable_stdio_usb(${ProjectName} 1)
pico_enable_stdio_uart(${ProjectName} 0)

target_include_directories(${ProjectName}
    PRIVATE src
    PRIVATE src/drivers
    PRIVATE src/tasks
    PRIVATE src/web
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/config
)

target_compile_definitions(${ProjectName} PRIVATE
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
    CYW43_HOST_NAME="pico2w"
)

target_link_libraries(${ProjectName}
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_http
    hardware_i2c
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(${ProjectName})
