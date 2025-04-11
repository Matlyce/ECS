add_library(ECS INTERFACE)

target_include_directories(ECS INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

if(WIN32)
    target_compile_definitions(ECS INTERFACE _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()