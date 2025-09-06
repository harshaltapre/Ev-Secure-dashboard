cmake_minimum_required(VERSION 3.16)

# Set the project name
set(PROJECT_NAME "evsecure-device")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set the project name
project(${PROJECT_NAME})

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Enable PSRAM
idf_build_set_property(COMPILE_OPTIONS "-DBOARD_HAS_PSRAM" APPEND)

# Enable Flash Encryption and Secure Boot
idf_build_set_property(COMPILE_OPTIONS "-DCONFIG_SECURE_FLASH_ENC_ENABLED=1" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DCONFIG_SECURE_BOOT=1" APPEND)
