#pragma once

#include "util/pch.h"
#include "util/core_config.h"


// FORWARD DECLARATIONS ================================================================================================


namespace PE::logger {

    // CONSTANTS =======================================================================================================

    // This enables the different log levels (FATAL + ERROR are always on)
    //  0 = FATAL + ERROR
    //  1 = FATAL + ERROR + WARN
    //  2 = FATAL + ERROR + WARN + INFO
    //  3 = FATAL + ERROR + WARN + INFO + DEBUG
    //  4 = FATAL + ERROR + WARN + INFO + DEBUG + TRACE
    #ifdef DEBUG
        #define LOG_LEVEL_ENABLED           			4
    #else
        #define LOG_LEVEL_ENABLED           			2
    #endif

    #ifndef FILE_LOG_LEVEL_ENABLED
        #define FILE_LOG_LEVEL_ENABLED LOG_LEVEL_ENABLED
    #endif

    // MACROS ==========================================================================================================

    #undef ERROR

    // TYPES ===========================================================================================================

    // Define the severity levels for logging
    // @note severity Enum representing the levels of logging severity
    // @note Trace The lowest level, used for tracing program execution
    // @note Debug Used for detailed debug information
    // @note Info Informational messages that highlight progress
    // @note Warn Messages for potentially harmful situations
    // @note Error Error events that might still allow the application to continue
    // @note Fatal Severe error events that lead to application shutdown
    enum class severity : u8 {
        trace = 0,
        debug,
        info,
        warn,
        error,
        fatal
    };

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================


    // Initialize the logging system
    // @param format The initial log message format
    // @param log_to_console should the log message be written to std::cout?
    // @param log_dir the directory that will contain all log files
    // @ main_log_file_name name of the central log_file (the thread that runs logger::init())
    // @param use_append_mode Should the system write over the existing log file or append to it
    bool init(const std::string& format, const bool log_to_console = false, const std::filesystem::path log_dir = "./logs", const std::string& main_log_file_name = "general.log", const bool use_append_mode = false);


    // Shuts down the logging subsystem: stops the worker thread, drains and processes
    // any remaining queued log messages, flushes buffered messages to the main log file,
    // and marks the logger as uninitialized.
    // If the logger was not initialized, an error is printed and the program exits immediately.
    // @return None.
    void shutdown();

    // Returns the filesystem path to the main log file used by the logger.
    // @return A std::filesystem::path pointing to the current main log file.
    std::filesystem::path get_log_file_location();


    // The format of log-messages can be customized with the following tags
    // @note to format all following log-messages use: set_format()
    // @note e.g. set_format("$B[$T] $L [$F] $C$E")
    //
    // @param $T time                    hh:mm:ss
    // @param $H hour                    hh
    // @param $M minute                  mm
    // @param $S secund                  ss
    // @param $J milliseconds            jjj
    //
    // @param $N data                    yyyy:mm:dd
    // @param $Y data year               yyyy
    // @param $O data month              mm
    // @param $D data day                dd
    //
    // @param $Q thread                  Thread_id: 137575225550656 or a label if provided
    // @param $F function name           application::main, math::foo
    // @param $P only function name      main, foo
    // @param $A file name               /home/workspace/test_cpp/src/main.cpp  /home/workspace/test_cpp/src/project.cpp
    // @param $I only file name          main.cpp
    // @param $G line                    1, 42
    //
    // @param $L log-level               add used log severity: [TRACE], [DEBUG] ... [FATAL]
    // @param $X alignment               adds space for "INFO" & "WARN"
    // @param $B color begin             from here the color begins
    // @param $E color end               from here the color will be reset
    // @param $C text                    the message the user wants to print
    // @param $Z new line                add a new line in the message format
    void set_format(const std::string& new_format);


    // Restore the previous log-message format
    // @note This function swaps the current log-message format with the previously stored backup.
    // It's useful for reverting to the previous format after temporary changes
    void use_previous_format();


    // Returns the current log output format string.
    // @return A copy of the format string used for log messages.
    const std::string get_format();


    // all messages with a lower severity than the provided argument will be buffered
    // Trace => buffer[]
    // Debug => buffer[Trace]
    // Info  => buffer[Trace + Debug]
    // Warn  => buffer[Trace + Debug + Info]
    // Error => buffer[Trace + Debug + Info + Warn]     (Error and Fatal will nover be buffered)
    // Fatal => buffer[Trace + Debug + Info + Warn]     (Error and Fatal will nover be buffered)
    void set_buffer_threshold(const severity new_threshold);


    // set the size of the buffer.
    // @note for messages that are not directly logged
    void set_buffer_size(const size_t new_size);


    // Registers a label for a specific thread, allowing for easier identification in logs.
    // If a label is already registered for the given thread ID, it will be overridden with the new label.
    // @param thread_label The label to be associated with the thread.
    // @param thread_id The ID of the thread for which the label is being registered.
    //                  Defaults to the ID of the calling thread if not provided.
    void register_label_for_thread(const std::string& thread_label, const std::thread::id thread_id = std::this_thread::get_id());


    // Unregisters the label for a specific thread, removing its association from the logger.
    // If no label is registered for the given thread ID, a message will be logged indicating that the operation was ignored.
    // @param thread_id The ID of the thread for which the label is being unregistered.
    //                  Defaults to the ID of the calling thread if not provided.
    void unregister_label_for_thread(std::thread::id thread_id = std::this_thread::get_id());


    // THIS SHOULD NEVER BE DIRECTLY CALLED
    // @note empty log messages will be ignored
    void log_msg_internal(const severity msg_sev, const std::source_location location_info, const std::thread::id thread_id, std::string message);

    // Template version that uses std::format for format strings with arguments
    template<typename... Args>
    inline void log_msg(const severity msg_sev, const std::source_location location_info, const std::thread::id thread_id, std::format_string<Args...> fmt, Args&&... args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        log_msg_internal(msg_sev, location_info, thread_id, std::move(message));
    }

    // Overload for plain strings (for backward compatibility)
    inline void log_msg(const severity msg_sev, const std::source_location location_info, const std::thread::id thread_id, const std::string& message) {
        log_msg_internal(msg_sev, location_info, thread_id, message);
    }

    inline void log_msg(const severity msg_sev, const std::source_location location_info, const std::thread::id thread_id, const char* message) {
        log_msg_internal(msg_sev, location_info, thread_id, std::string(message));
    }

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================

    // An exception type that logs the error message immediately when constructed.
    // The exception stores the provided message and also forwards it to the logger
    // with context (location_info, thread).
    // @note This class inherits from std::exception so it can be thrown/caught like a standard exception.
    class logged_exception : public std::exception {
		public:

            // Constructs a logged_exception from source location, thread id and a string message.
            template<typename... Args>
			explicit logged_exception(const std::source_location location_info, const std::thread::id thread_id, 
                std::format_string<Args...> fmt, Args&&... args)
            : m_msg(std::format(fmt, std::forward<Args>(args)...)) {
                logger::log_msg_internal(logger::severity::error, location_info, thread_id, m_msg);
            }

            // Overload for plain string
            explicit logged_exception(const std::source_location location_info, const std::thread::id thread_id, const std::string& message)
                : m_msg(message) {
                logger::log_msg_internal(logger::severity::error, location_info, thread_id, m_msg);
            }

            // Overload for C-string
            explicit logged_exception(const std::source_location location_info, const std::thread::id thread_id, const char* message)
                : m_msg(message) {
                logger::log_msg_internal(logger::severity::error, location_info, thread_id, m_msg);
            }

            // Returns a C-string describing the exception. Marked noexcept to match std::exception::what().
            // @return A pointer to a null-terminated C-string containing the stored error message.
            virtual const char* what() const noexcept override { return m_msg.c_str(); }

		private:

            // The stored error message for this exception instance.
            // @note This string is the source for the pointer returned by what().
			std::string m_msg;
	};

}

// MACROS ==============================================================================================================

// Logger support macros -----------------------------------------------------------------------------------------------
//      split macros into severity specific macros that use all the same master to enable severity level specific logging,
//      this will remove unused logs at compile time depending on the severity

#define LOG_Master(severity_level, fmt, ...)                                                                            \
    {                                                                                                                   \
        PE::logger::log_msg(PE::logger::severity::severity_level, std::source_location::current(),                      \
            std::this_thread::get_id(), fmt __VA_OPT__(,) __VA_ARGS__);                                                 \
    }

#define LOG_fatal(fmt, ...)          LOG_Master(fatal, fmt __VA_OPT__(,) __VA_ARGS__)
#define LOG_error(fmt, ...)          LOG_Master(error, fmt __VA_OPT__(,) __VA_ARGS__)

#if FILE_LOG_LEVEL_ENABLED > 0
    #define LOG_warn(fmt, ...)       LOG_Master(warn, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define LOG_warn(fmt, ...)       { }
#endif

#if FILE_LOG_LEVEL_ENABLED > 1
    #define LOG_info(fmt, ...)       LOG_Master(info, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define LOG_info(fmt, ...)       { }
#endif

#if FILE_LOG_LEVEL_ENABLED > 2
    #define LOG_debug(fmt, ...)      LOG_Master(debug, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define LOG_debug(fmt, ...)      { }
#endif

#if FILE_LOG_LEVEL_ENABLED > 3
    #define LOG_trace(fmt, ...)      LOG_Master(trace, fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define LOG_trace(fmt, ...)      { }
#endif


// Logger main macro ---------------------------------------------------------------------------------------------------

#define LOG(severity, fmt, ...)      LOG_##severity(fmt __VA_OPT__(,) __VA_ARGS__)


// util ----------------------------------------------------------------------------------------------------------------

#define LOGGED_EXCEPTION(fmt, ...)                                                                                      \
    {                                                                                                                   \
        throw PE::logger::logged_exception(std::source_location::current(), std::this_thread::get_id(),                 \
            "LOGGER EXCEPTION: " fmt __VA_OPT__(,) __VA_ARGS__);                                                        \
    }

#define LOG_INIT                                                            LOG(trace, "init");
#define LOG_SHUTDOWN                                                        LOG(trace, "shutdown");

// Assertion & Validation ----------------------------------------------------------------------------------------------
// In logger.h

// For ASSERT macro
#if defined (PLATFORM_WINDOWS)
    #if ENABLE_LOGGING_FOR_ASSERTS
        #define ASSERT(expr, message_success, message_failure, ...)                                                     \
            if (expr)                                                                                                   \
                LOG(trace, message_success __VA_OPT__(,) __VA_ARGS__)                                                   \
            else {                                                                                                      \
                LOG(fatal, message_failure __VA_OPT__(,) __VA_ARGS__)                                                   \
                DEBUG_BREAK();                                                                                          \
            }

        #define ASSERT_S(expr)                                                                                          \
            if (!(expr)) {                                                                                              \
                LOG(fatal, "Assertion failed: {}", #expr)                                                               \
                DEBUG_BREAK();                                                                                          \
            }
    #else
        #define ASSERT(expr, message_success, message_failure, ...)        if (!(expr)) { DEBUG_BREAK(); }
        #define ASSERT_S(expr)                                              if (!(expr)) { DEBUG_BREAK(); }
    #endif

#elif defined (PLATFORM_LINUX)

    #if ENABLE_LOGGING_FOR_ASSERTS
        #define ASSERT(expr, message_success, message_failure, ...)                                                     \
            if (expr)                                                                                                   \
                LOG(trace, message_success __VA_OPT__(,) __VA_ARGS__)                                                   \
            else {                                                                                                      \
                LOG(fatal, message_failure __VA_OPT__(,) __VA_ARGS__)                                                   \
                LOGGED_EXCEPTION(message_failure __VA_OPT__(,) __VA_ARGS__);                                            \
            }

        #define ASSERT_S(expr)                                                                                          \
            if (!(expr)) {                                                                                              \
                LOG(fatal, "Assertion failed: {}", #expr)                                                               \
                LOGGED_EXCEPTION("Assertion failed: {}", #expr);                                                        \
            }
    #else
        #define ASSERT(expr, message_success, message_failure, ...)        if (!(expr)) { LOGGED_EXCEPTION("Assertion failed: {}", #expr); }
        #define ASSERT_S(expr)                                              if (!(expr)) { LOGGED_EXCEPTION("Assertion failed: {}", #expr); }
    #endif

#endif

// For VALIDATE macro
#if ENABLE_LOGGING_FOR_VALIDATION
    #define VALIDATE(expr, command, message_success, message_failure, ...)                                              \
        if (expr) {                                                                                                     \
            LOG(trace, message_success __VA_OPT__(,) __VA_ARGS__)                                                       \
        } else {                                                                                                        \
            LOG(error, message_failure __VA_OPT__(,) __VA_ARGS__)                                                       \
            command;                                                                                                    \
        }

    #define VALIDATE_S(expr, command)                                                                                   \
        if (!(expr)) {                                                                                                  \
            LOG(error, "Validation failed: {}", #expr)                                                                  \
            command;                                                                                                    \
        }
#else
    #define VALIDATE(expr, command, message_success, message_failure, ...)  if (!(expr)) { command; }
    #define VALIDATE_S(expr, command)                                       if (!(expr)) { command; }
#endif
