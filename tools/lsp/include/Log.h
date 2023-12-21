#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

class Log {
public:
    enum LogVerbosity { Off, Low, Medium, High, Debug };

    static void setVerbosity(LogVerbosity v) { verbosity = v; }

    static void useColors(bool b) { colors = b; }

    template<typename... Args>
    static void warning(std::string_view fmt, Args&&... args) {
        printTime();
        fmt::vprint(stderr, fg(fmt::color::yellow), fmt,
                    fmt::make_format_args(std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void error(std::string_view fmt, Args&&... args) {
        printTime();
        fmt::vprint(stderr, fg(fmt::color::red), fmt,
                    fmt::make_format_args(std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void fatal(std::string_view fmt, Args&&... args) {
        printTime();
        fmt::vprint(stderr, fg(fmt::color::red), fmt,
                    fmt::make_format_args(std::forward<Args>(args)...));
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

    static inline bool colors = false;

    template<typename... Args>
    static void message(std::string_view fmt, LogVerbosity v, Args&&... args) {
        if (v <= verbosity) {
            printTime();
            if (colors)
                fmt::vprint(stderr, fg(fmt::color::green), fmt,
                            fmt::make_format_args(args...));
            else
                fmt::vprint(stderr, fmt, fmt::make_format_args(args...));
        }
    }

    static void printTime() { fmt::print(stderr, "[{:%F %T}] ", std::chrono::system_clock::now()); }
};