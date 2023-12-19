#include "IOManager.h"

#include <filesystem>

namespace
{
	// TODO: This only works with the expectation that the root folder is called Poly - which would only be true for this project.
	//		 Make this project-agnostic by either having the user provide a path, or magically find it
	constexpr std::string_view k_PolyPath = "Poly/";
}

namespace Poly
{
	std::string IOManager::GetRootFolder()
	{
		const std::string currentPath = std::filesystem::current_path().string();
		const size_t polyPos = currentPath.rfind(k_PolyPath);
		return currentPath.substr(0, polyPos + k_PolyPath.length());
	}

	std::string IOManager::GetAssetsFolder()
	{
		return GetRootFolder() += "assets/";
	}

	std::string IOManager::GetScenesFolder()
	{
		return GetRootFolder() += "assets/scenes/";
	}

	bool IOManager::FileExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}
}
