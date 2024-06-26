#pragma once
#pragma 

#pragma warning(push)
#pragma warning( disable : 26439 26451 26495 26439 26451 26495 26812 6385 26498 26450 26437 6285)
#define FMT_USE_USER_DEFINED_LITERALS 0
#include "spdlog/spdlog.h"
#pragma warning(pop)


#pragma warning(push)
#pragma warning(disable:4251)

namespace Poly {

	class Logger
	{
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger> getCoreLogger() { return coreLogger; }
		inline static std::shared_ptr<spdlog::logger> getClientLogger() { return clientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> coreLogger;
		static std::shared_ptr<spdlog::logger> clientLogger;
	};
}

// Poly logger macros
// #define POLY_CORE_TRACE(...) ::Poly::Logger::getCoreLogger()->trace(__VA_ARGS__)
// #define POLY_CORE_INFO(...) ::Poly::Logger::getCoreLogger()->info(__VA_ARGS__)
// #define POLY_CORE_WARN(...) ::Poly::Logger::getCoreLogger()->warn(__VA_ARGS__)
// #define POLY_CORE_ERROR(...) ::Poly::Logger::getCoreLogger()->error(__VA_ARGS__)
// #define POLY_CORE_FATAL(...) ::Poly::Logger::getCoreLogger()->critical(__VA_ARGS__)

#define POLY_CORE_TRACE(...) SPDLOG_LOGGER_CALL(::Poly::Logger::getCoreLogger(), spdlog::level::trace, __VA_ARGS__)
#define POLY_CORE_INFO(...) SPDLOG_LOGGER_CALL(::Poly::Logger::getCoreLogger(), spdlog::level::info, __VA_ARGS__)
#define POLY_CORE_WARN(...) SPDLOG_LOGGER_CALL(::Poly::Logger::getCoreLogger(), spdlog::level::warn, __VA_ARGS__)
#define POLY_CORE_ERROR(...) SPDLOG_LOGGER_CALL(::Poly::Logger::getCoreLogger(), spdlog::level::err, __VA_ARGS__)
#define POLY_CORE_FATAL(...) SPDLOG_LOGGER_CALL(::Poly::Logger::getCoreLogger(), spdlog::level::critical, __VA_ARGS__)

// Client logger macros
#define POLY_TRACE(...) ::Poly::Logger::getClientLogger()->trace(__VA_ARGS__)
#define POLY_INFO(...) ::Poly::Logger::getClientLogger()->info(__VA_ARGS__)
#define POLY_WARN(...) ::Poly::Logger::getClientLogger()->warn(__VA_ARGS__)
#define POLY_ERROR(...) ::Poly::Logger::getClientLogger()->error(__VA_ARGS__)
#define POLY_FATAL(...) ::Poly::Logger::getClientLogger()->critical(__VA_ARGS__)


#pragma warning(pop)