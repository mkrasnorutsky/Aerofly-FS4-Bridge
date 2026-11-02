# Test configuration for Aerofly FS4 Bridge
cmake_minimum_required(VERSION 3.15)

# Fetch Catch2 testing framework
message(STATUS "Fetching Catch2 testing framework...")
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

# Add Catch2 CMake modules to path
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

# Create main test executable with all test files
add_executable(AeroflyBridge_Tests
    test_helpers.h
    ../src/logging/logger.cpp
    unit/test_variable_mapper.cpp
    unit/test_json_builder.cpp
    unit/test_command_processor.cpp
    integration/test_shared_memory.cpp
    integration/test_tcp_server.cpp
    integration/test_websocket_server.cpp
)

# Include directories for tests
target_include_directories(AeroflyBridge_Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${AEROFLY_SDK_PATH}
)

# Link against Catch2 and AeroflyBridge
target_link_libraries(AeroflyBridge_Tests PRIVATE
    Catch2::Catch2WithMain
    spdlog::spdlog_header_only
    ws2_32
    kernel32
    user32
)

# Compiler definitions for tests
target_compile_definitions(AeroflyBridge_Tests PRIVATE
    WIN32
    _WINDOWS
)

# Copy AeroflyBridge.dll to test directory after build
add_custom_command(TARGET AeroflyBridge_Tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:AeroflyBridge>
        $<TARGET_FILE_DIR:AeroflyBridge_Tests>
    COMMENT "Copying AeroflyBridge.dll to test directory"
)

# Enable CTest integration
include(CTest)
include(Catch)
catch_discover_tests(AeroflyBridge_Tests)

# Custom target to run tests with verbose output
add_custom_target(run_tests
    COMMAND AeroflyBridge_Tests --reporter console
    DEPENDS AeroflyBridge_Tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests..."
)

message(STATUS "Test configuration complete - ${CMAKE_CURRENT_LIST_FILE}")
