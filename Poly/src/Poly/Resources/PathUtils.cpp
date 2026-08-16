#include "PathUtils.h"

#include <filesystem>

namespace Poly::PathUtils
{
	std::string GetFileName(std::string_view path)
	{
		return std::filesystem::path(path).filename().string();
	}

	std::string GetDirectoryPath(std::string_view path)
	{
		return std::filesystem::path(path).parent_path().string();
	}

	std::string GetExtension(std::string_view path)
	{
		return std::filesystem::path(path).extension().string();
	}
} // namespace Poly::PathUtils