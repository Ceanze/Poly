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

	std::string_view GetSourcePath(std::string_view path)
	{
		return path.substr(0, path.find('#'));
	}
} // namespace Poly::PathUtils