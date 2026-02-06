# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(MonitorAmbiental C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# FreeRTOS
set(FREERTOS_KERNEL_PATH "${PICO_SDK_PATH}/lib/FreeRTOS-Kernel" CACHE STRING "FreeRTOS Kernel Path")
if (EXISTS "${FREERTOS_KERNEL_PATH}/CMakeLists.txt")
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config SYSTEM
        INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/include
            ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040
            ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
    )
    target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0 configUSE_EVENT_GROUPS=1)

    set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include CACHE STRING "FreeRTOS config directory")
    set(FREERTOS_PORT GCC_RP2040 CACHE STRING "FreeRTOS Port")
    set(FREERTOS_HEAP 4 CACHE STRING "FreeRTOS Heap implementation")
    add_subdirectory(${FREERTOS_KERNEL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/freertos_kernel)

else()
    message(FATAL_ERROR "FreeRTOS-Kernel nao encontrado em ${FREERTOS_KERNEL_PATH}")
endif()

# Add executable. Default name is the project name, version 0.1

add_executable(MonitorAmbiental 
    src/MonitorAmbiental.c 
    drivers/ssd1306.c
    drivers/bh1750.c
    drivers/aht10.c
    drivers/led_matrix.c
    src/sensor_data.c
    src/wifi_manager.c
    web/web_server.c
    web/auth.c
    web/web_pages.c
    src/rtos/rtos_app.c
    src/rtos/task_sensors.c
    src/rtos/task_display.c
    src/rtos/task_uart.c
    src/rtos/task_web.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

# Generate PIO header
pico_generate_pio_header(MonitorAmbiental ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

pico_set_program_name(MonitorAmbiental "MonitorAmbiental")
pico_set_program_version(MonitorAmbiental "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(MonitorAmbiental 1)
pico_enable_stdio_usb(MonitorAmbiental 1)

# Add the standard library to the build
target_link_libraries(MonitorAmbiental
    pico_stdlib
    freertos_kernel
    freertos_config)

# Add the standard include files to the build
target_include_directories(MonitorAmbiental PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/drivers
    ${CMAKE_CURRENT_LIST_DIR}/web
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
)

# Add any user requested libraries
target_link_libraries(MonitorAmbiental 
        hardware_i2c
        hardware_pio
        pico_cyw43_arch_lwip_threadsafe_background
        )

target_compile_definitions(MonitorAmbiental PRIVATE
    USE_FREERTOS=1
    )

pico_add_extra_outputs(MonitorAmbiental)

