add_executable(${TTA_SRV_NAME} main.cpp)
target_link_libraries(${TTA_SRV_NAME} PRIVATE ${TTA_LIB_NAME})

set_property(TARGET ${TTA_SRV_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
 
add_custom_command(
        TARGET ${TTA_SRV_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/service/tta-service.cfg
                ${CMAKE_CURRENT_BINARY_DIR}/tta-service.cfg)

 install(
  TARGETS ${TTA_LIB_NAME}
  RUNTIME DESTINATION bin
)