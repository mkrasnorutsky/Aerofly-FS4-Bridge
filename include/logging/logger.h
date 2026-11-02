///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file logger.h
 * @brief Logging system wrapper for Aerofly FS4 Bridge
 *
 * Provides a structured logging interface with multiple log levels and outputs.
 * Based on spdlog for high-performance async logging.
 *
 * Features:
 * - Multiple log levels (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL)
 * - Console output (OutputDebugString on Windows)
 * - File output with automatic rotation
 * - Thread-safe async logging
 * - Configurable via environment variables
 *
 * Environment Variables:
 * - AEROFLY_BRIDGE_LOG_LEVEL   : Set minimum log level (trace|debug|info|warn|error|critical)
 * - AEROFLY_BRIDGE_LOG_FILE    : Enable file logging (0|1, default: 1)
 * - AEROFLY_BRIDGE_LOG_CONSOLE : Enable console logging (0|1, default: 1)
 *
 * Usage:
 * @code
 *   #include "logging/logger.h"
 *
 *   // In main initialization
 *   Logger::Initialize();
 *
 *   // Throughout the code
 *   LOG_INFO("Server started on port {}", port);
 *   LOG_DEBUG("Processing message: {}", msg);
 *   LOG_ERROR("Failed to connect: error code {}", err);
 *
 *   // On shutdown
 *   Logger::Shutdown();
 * @endcode
 */
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>

// Forward declare spdlog types to avoid including heavy headers
namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

namespace AeroflyBridge {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Logger Class - Main logging interface
///////////////////////////////////////////////////////////////////////////////////////////////////

class Logger {
public:
    /**
     * @brief Initialize the logging system
     *
     * Sets up console and file sinks based on environment variables.
     * This should be called once at application startup.
     *
     * @return true if initialization succeeded, false otherwise
     */
    static bool Initialize();

    /**
     * @brief Shutdown the logging system
     *
     * Flushes all pending log messages and releases resources.
     * Call this before application exit.
     */
    static void Shutdown();

    /**
     * @brief Get the internal logger instance
     *
     * @return Shared pointer to the spdlog logger
     */
    static std::shared_ptr<spdlog::logger> GetLogger();

    /**
     * @brief Flush all pending log messages
     *
     * Forces immediate write of all buffered log messages.
     * Useful before critical operations or shutdown.
     */
    static void Flush();

    /**
     * @brief Check if logger is initialized
     *
     * @return true if logger is ready to use
     */
    static bool IsInitialized();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;

    // Helper to get log file path
    static std::string GetLogFilePath();

    // Helper to parse log level from environment variable
    static int GetLogLevelFromEnv();
};

} // namespace AeroflyBridge

///////////////////////////////////////////////////////////////////////////////////////////////////
// Convenience Macros for Logging
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(NDEBUG)
    // In Release builds, TRACE and DEBUG are disabled
    #define LOG_TRACE(...)    ((void)0)
    #define LOG_DEBUG(...)    ((void)0)
#else
    // In Debug builds, all levels are available
    #define LOG_TRACE(...)    if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->trace(__VA_ARGS__)
    #define LOG_DEBUG(...)    if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->debug(__VA_ARGS__)
#endif

#define LOG_INFO(...)         if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)         if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)        if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)     if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::GetLogger()->critical(__VA_ARGS__)

// Flush macro for critical sections
#define LOG_FLUSH()           if(::AeroflyBridge::Logger::IsInitialized()) ::AeroflyBridge::Logger::Flush()
