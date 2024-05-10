#pragma once

#ifdef POLY_PLATFORM_WINDOWS
#include <format>
#else
#include <spdlog/fmt/bundled/format.h>
#endif

namespace Poly {
#ifdef POLY_PLATFORM_WINDOWS
    template <typename... T>
    inline auto Format(std::format_string<T...> format, T&&... args)
    {
        return std::format(format, std::forward<T>(args)...);
    }
#else
    template <typename... T>
    inline auto Format(fmt::format_string<T...> format, T&&... args)
    {
        return fmt::format(format, std::forward<T>(args)...);
    }
#endif
}
