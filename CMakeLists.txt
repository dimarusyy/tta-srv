cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

set(TTA_SRV_VERSION 0.0.1)
set(TTA_SRV_NAME "tta-service")

project(tta-srv LANGUAGES CXX
                VERSION "${TTA_SRV_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(dependencies)

add_subdirectory(src)

#enable_testing()
#add_subdirectory(tests)

