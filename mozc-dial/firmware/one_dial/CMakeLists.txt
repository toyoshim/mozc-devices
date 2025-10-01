# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
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
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(one_dial C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(one_dial
        ../common/dial_controller.cc
        ../common/dial_controller.h
        ../common/motor_controller.cc
        ../common/motor_controller.h
        ../common/photo_sensor.cc
        ../common/photo_sensor.h
        ../common/usage_tables.cc
        ../common/usage_tables.h
        ../common/usb_device.cc
        ../common/usb_device.h
        ../common/usb_hid_device.cc
        ../common/usb_hid_device.h
        ../common/usb_hid_keyboard.cc
        ../common/usb_hid_keyboard.h
        one_dial.cc
        )

pico_set_program_name(one_dial "one_dial")
pico_set_program_version(one_dial "0.1")

# Modify the below lines to enable/disable output over UART/USB/SEMIHOSTING
pico_enable_stdio_uart(one_dial 1)
pico_enable_stdio_usb(one_dial 0)
pico_enable_stdio_semihosting(one_dial 0)

# Add the standard library to the build
target_link_libraries(one_dial
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(one_dial PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(one_dial)
