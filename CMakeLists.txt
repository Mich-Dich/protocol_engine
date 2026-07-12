
cmake_minimum_required(VERSION                  3.30)

set(PROJECT_NAME "protocol_engine")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD                          26)
set(CMAKE_CXX_STANDARD_REQUIRED                 ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS               ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY              ${CMAKE_SOURCE_DIR}/build/bin)


file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
add_executable(${PROJECT_NAME}                  ${SOURCES})

if(UNIX AND NOT APPLE)
    add_compile_definitions(PLATFORM_LINUX)
    set(LINUX_SYSTEM_LIBS                       pthread dl) # set a variable for system libraries
elseif(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
    if(MSVC)                                    # Apply /bigobj to all targets (valid only for MSVC)
        add_compile_options(/bigobj)
    endif()
elseif(APPLE)
    message(FATAL_ERROR "Apple/macOS platform is not supported by this project.")
endif()

add_compile_definitions(DEBUG)

target_precompile_headers(${PROJECT_NAME}       PRIVATE
    "${CMAKE_SOURCE_DIR}/src/util/pch.h"
)

target_include_directories(${PROJECT_NAME}      PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_compile_options(${PROJECT_NAME}          PRIVATE
    -std=c++26
    -freflection
)

option(CORE_VERBOSE_INFO                            "Show detailed file list for Core" OFF)
set(CORE_VERBOSE_INFO                               TRUE)
include(${CMAKE_SOURCE_DIR}/helper.cmake)
print_source_file_details(
    SOURCES                                             ${SOURCES}
    NAME                                                ${PROJECT_NAME}
    SOURCE_DIR                                          "${CMAKE_SOURCE_DIR}"
    VERBOSE                                             CORE_VERBOSE_INFO
    TARGET                                              ${PROJECT_NAME}
)
