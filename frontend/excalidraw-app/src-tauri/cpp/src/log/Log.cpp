//
// Created by 박성빈 on 25. 6. 11.
//

#include "Log.h"
#include <iostream>

namespace Log {
    namespace detail {
        void print_with_tag(const Level tag, const std::string_view message) {
            std::ostream& out = tag == Level::ERROR ? std::cerr : std::cout;
            out << "[" << level_to_string(tag) << "] " << message << '\n' << std::flush;
        }

        std::string level_to_string(Level l) {
            const auto idx = static_cast<int>(l);
            if (idx < 0 || idx >= static_cast<int>(Level::UNKNOWN)) return "UNKNOWN";
            return levelNames[idx];
        }
    }

    void info(const std::string_view message) {
        detail::print_with_tag(Level::INFO, message);
    }

    void debug(const std::string_view message) {
        detail::print_with_tag(Level::DEBUG, message);
    }

    void warn(const std::string_view message) {
        detail::print_with_tag(Level::WARN, message);
    }

    void error(const std::string_view message) {
        detail::print_with_tag(Level::ERROR, message);
    }


}
