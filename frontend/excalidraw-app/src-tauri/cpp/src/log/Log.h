//
// Created by 박성빈 on 25. 6. 11.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <format>


namespace Log {

    enum class Level {
        INFO,
        DEBUG,
        WARN,
        ERROR,
        UNKNOWN
    };

    namespace detail {
        void print_with_tag(std::string_view tag, std::string_view message);

        template<typename... Args>
        std::string fmt(const std::string_view format, const Args&... args) {
            return std::vformat(format, std::make_format_args(args...));
        }

        constexpr std::array<std::string, 4> levelNames = {
            "INFO", "DEBUG", "WARN", "ERROR",
        };

        std::string level_to_string(Level l);
    }

    void info(std::string_view message);
    void debug(std::string_view message);
    void warn(std::string_view message);
    void error(std::string_view message);

    template<typename... Args>
    void info(const std::string_view format, const Args&... args) {
        info(detail::fmt(format, args...));
    }

    template<typename... Args>
    void debug(const std::string_view format, const Args&... args) {
        debug(detail::fmt(format, args...));
    }

    template<typename... Args>
    void warn(const std::string_view format, const Args&... args) {
        warn(detail::fmt(format, args...));
    }

    template<typename Exception, typename... Args>
    void error(const std::string_view format, const Args&... args) {
        const std::string message = detail::fmt(format, args...);
        error(message);
        throw Exception(message);
    }

};



#endif //LOGGER_H
