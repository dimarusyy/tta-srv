cmake_minimum_required(VERSION 3.12)

file(GLOB_RECURSE TTA_SRV_HEADERS $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.12>:CONFIGURE_DEPENDS>
    ${CMAKE_SOURCE_DIR}/src/*.h
)

file(GLOB_RECURSE TTA_SRV_SOURCES $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.12>:CONFIGURE_DEPENDS>
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(${TTA_SRV_NAME} ${TTA_SRV_HEADERS} ${TTA_SRV_SOURCES})
target_include_directories(${TTA_SRV_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${TTA_SRV_NAME} PRIVATE tomlplusplus::tomlplusplus spdlog::spdlog)

add_custom_command(
        TARGET ${TTA_SRV_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/tta-service.cfg
                ${CMAKE_CURRENT_BINARY_DIR}/tta-service.cfg)