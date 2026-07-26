cmake_minimum_required(VERSION 3.21)

# Nama project harus sama kayak "id" belakang di mod.json (tanpa titik)
project(BOTPLAYER VERSION 1.0.0)

# Semua file .cpp di folder src/ otomatis kebaca
file(GLOB SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Ambil Geode SDK. Kalau belum ada di system, CMake bakal download otomatis
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable belum di-set! Install Geode CLI dulu: https://docs.geode-sdk.org")
else()
    message(STATUS "Geode SDK ditemukan di: $ENV{GEODE_SDK}")
    add_subdirectory($ENV{GEODE_SDK} ${CMAKE_BINARY_DIR}/geode)
endif()

setup_geode_mod(${PROJECT_NAME})
