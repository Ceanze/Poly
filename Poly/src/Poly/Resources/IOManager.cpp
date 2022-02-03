#include "IOManager.h"

#include <filesystem>

namespace Poly
{
	bool IOManager::FileExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}
}