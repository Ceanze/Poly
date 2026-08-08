#include "PathUtils.h"

namespace Poly::PathUtils
{
	std::string GetFileName(std::string_view path)
	{
		size_t slashPos = path.find_last_of("/\\");
		if (slashPos == std::string_view::npos)
			return std::string(path);

		return std::string(path).substr(slashPos + 1);
	}

	std::string GetDirectoryPath(std::string_view path)
	{
		size_t slashPos = path.find_last_of("/\\");
		if (slashPos == std::string_view::npos)
			return std::string(path);

		return std::string(path).substr(0, slashPos);
	}

	std::string GetExtension(std::string_view path)
	{
		size_t dotPos = path.find_last_of(".");
		if (dotPos == std::string_view::npos)
			return "";

		return std::string(path).substr(dotPos + 1);
	}
} // namespace Poly::PathUtils