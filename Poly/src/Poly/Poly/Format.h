#pragma once

#include <spdlog/fmt/bundled/format.h>

namespace Poly {
    template <typename... T>
    inline auto Format(const std::string& format, T&&... args)
    {
#ifdef POLY_WINDOWS
        return std::format(format, std::forward(args...));
#else
        return fmt::format(format, std::forward<T>(args)...);
#endif
    }
}
