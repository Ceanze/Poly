#pragma once
#pragma 

#pragma warning(push)
#pragma warning( disable : 26439 26451 26495 26439 26451 26495 26812 6385)
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
#define POLY_CORE_TRACE(...) ::Poly::Logger::getCoreLogger()->trace(__VA_ARGS__)
#define POLY_CORE_INFO(...) ::Poly::Logger::getCoreLogger()->info(__VA_ARGS__)
#define POLY_CORE_WARN(...) ::Poly::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define POLY_CORE_ERROR(...) ::Poly::Logger::getCoreLogger()->error(__VA_ARGS__)
#define POLY_CORE_FATAL(...) ::Poly::Logger::getCoreLogger()->critical(__VA_ARGS__)

// Client logger macros
#define POLY_TRACE(...) ::Poly::Logger::getClientLogger()->trace(__VA_ARGS__)
#define POLY_INFO(...) ::Poly::Logger::getClientLogger()->info(__VA_ARGS__)
#define POLY_WARN(...) ::Poly::Logger::getClientLogger()->warn(__VA_ARGS__)
#define POLY_ERROR(...) ::Poly::Logger::getClientLogger()->error(__VA_ARGS__)
#define POLY_FATAL(...) ::Poly::Logger::getClientLogger()->critical(__VA_ARGS__)


#pragma warning(pop)