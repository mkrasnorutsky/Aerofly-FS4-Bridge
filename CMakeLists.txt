cmake_minimum_required(VERSION 3.15)

# Project declaration
project(AeroflyBridge
    VERSION 0.4.1
    DESCRIPTION "Multi-interface bridge (Shared Memory, TCP, WebSocket) for Aerofly FS4"
    LANGUAGES CXX
)

# C++17 is required for this project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTING "Build the testing tree" ON)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# Include FetchContent module for downloading dependencies
include(FetchContent)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Fetch spdlog for logging
message(STATUS "Fetching spdlog...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

# Find Aerofly SDK header (tm_external_message.h)
# Users can set AEROFLY_SDK_PATH to override search
if(NOT DEFINED AEROFLY_SDK_PATH)
    # Search in common locations
    set(AEROFLY_SDK_SEARCH_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "$ENV{AEROFLY_SDK_PATH}"
    )

    foreach(search_path ${AEROFLY_SDK_SEARCH_PATHS})
        if(EXISTS "${search_path}/tm_external_message.h")
            set(AEROFLY_SDK_PATH "${search_path}")
            message(STATUS "Found Aerofly SDK header at: ${AEROFLY_SDK_PATH}")
            break()
        endif()
    endforeach()

    if(NOT DEFINED AEROFLY_SDK_PATH)
        message(WARNING
            "tm_external_message.h not found. Please:\n"
            "  1. Download from https://www.aerofly.com/developers/\n"
            "  2. Place in project root, or\n"
            "  3. Set AEROFLY_SDK_PATH to the directory containing it\n"
            "Build will continue but compilation will fail without this header."
        )
        set(AEROFLY_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endif()

# Main DLL library target
add_library(AeroflyBridge SHARED
    aerofly_bridge_dll.cpp
    src/logging/logger.cpp
)

# Include directories
target_include_directories(AeroflyBridge PRIVATE
    ${AEROFLY_SDK_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(AeroflyBridge PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Release>:NDEBUG>
)

# Compiler options
if(MSVC)
    target_compile_options(AeroflyBridge PRIVATE
        /EHsc                           # Exception handling model
        $<$<CONFIG:Release>:/O2>        # Optimization level for Release
        $<$<CONFIG:Release>:/GL>        # Whole program optimization
    )

    # Linker options for Release
    target_link_options(AeroflyBridge PRIVATE
        $<$<CONFIG:Release>:/LTCG>      # Link-time code generation
        $<$<CONFIG:Release>:/OPT:REF>   # Eliminate unreferenced functions
        $<$<CONFIG:Release>:/OPT:ICF>   # Identical COMDAT folding
    )

    # Enable warnings if requested
    if(ENABLE_WARNINGS)
        target_compile_options(AeroflyBridge PRIVATE
            /W4                          # Warning level 4
            /WX-                         # Warnings as errors (disabled for now)
        )
    endif()
else()
    message(WARNING "This project is designed for MSVC (Visual Studio). Other compilers may not work correctly.")
endif()

# Link against required Windows libraries and external dependencies
target_link_libraries(AeroflyBridge PRIVATE
    ws2_32      # Winsock 2
    kernel32    # Windows kernel
    user32      # Windows user32
    spdlog::spdlog_header_only  # Logging library
    nlohmann_json::nlohmann_json
)

# Set output directory
set_target_properties(AeroflyBridge PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

# Installation rules
install(TARGETS AeroflyBridge
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

# Install to Aerofly FS4 directory (optional, user convenience)
if(WIN32)
    set(AEROFLY_INSTALL_PATH "$ENV{USERPROFILE}/Documents/Aerofly FS 4/external_dll")
    if(EXISTS "${AEROFLY_INSTALL_PATH}")
        install(TARGETS AeroflyBridge
            RUNTIME DESTINATION "${AEROFLY_INSTALL_PATH}"
            LIBRARY DESTINATION "${AEROFLY_INSTALL_PATH}"
            OPTIONAL
        )
        message(STATUS "Optional install target configured for: ${AEROFLY_INSTALL_PATH}")
    endif()
endif()

# Testing configuration
if(BUILD_TESTING)
    enable_testing()
    add_executable(CommandContractTests tests/command_contract.cpp)
    target_include_directories(CommandContractTests PRIVATE include ${AEROFLY_SDK_PATH})
    target_link_libraries(CommandContractTests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME command_contract COMMAND CommandContractTests)
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Aerofly Bridge v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "SDK Path:          ${AEROFLY_SDK_PATH}")
message(STATUS "Build Testing:     ${BUILD_TESTING}")
message(STATUS "Enable Warnings:   ${ENABLE_WARNINGS}")
message(STATUS "========================================")
message(STATUS "")
