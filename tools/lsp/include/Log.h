#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <thread>

class Log {
public:
    enum LogVerbosity : uint8_t { Off, Low, Medium, High, Debug };

    static void setVerbosity(LogVerbosity v) { verbosity = v; }
    static void setOutput(std::FILE* out) { output = out; }

    static void useColors(bool b) { colors = b; }

    template<typename... Args>
    static void warning(std::string_view fmt, Args&&... args) {
        message(fmt, fg(fmt::color::yellow), Low, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(std::string_view fmt, Args&&... args) {
        message(fmt, fg(fmt::color::red), Low, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void fatal(std::string_view fmt, Args&&... args) {
        message(fmt, fg(fmt::color::red), Low, std::forward<Args>(args)...);
        exit(1);
    }

    template<typename... Args>
    static void low(std::string_view fmt, Args&&... args) {
        message(fmt, Low, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void medium(std::string_view fmt, Args&&... args) {
        message(fmt, Medium, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void high(std::string_view fmt, Args&&... args) {
        message(fmt, High, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(std::string_view fmt, Args&&... args) {
        message(fmt, Debug, std::forward<Args>(args)...);
    }

private:
    Log() = default;

    static inline LogVerbosity verbosity = Low;
    static inline std::FILE* output = stderr;
    static inline bool colors = false;

    template<typename... Args>
    static void message(std::string_view fmt, LogVerbosity v, Args&&... args) {
        message(fmt, fmt::text_style(), v, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void message(std::string_view fmt, fmt::text_style style, LogVerbosity v,
                        Args&&... args) {
        if (v <= verbosity) {
            printTime();
            auto used_style = colors ? style : fmt::text_style();
            fmt::vprint(output, used_style, fmt, fmt::make_format_args(args...));
            // Sleeping here so each message is displayed in a new line in the LSP client log
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    static void printTime() { fmt::print(output, "[{:%F %T}] ", std::chrono::system_clock::now()); }
};