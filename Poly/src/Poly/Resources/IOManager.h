#pragma once

namespace Poly
{
	class IOManager
	{
	private:
		// Standalone build should follow same structure
		static constexpr const char* ASSETS_FOLDER_PATH = "../assets/";

	public:
		IOManager() = default;
		~IOManager() = default;

		static std::string GetAssetsFolder() { return ASSETS_FOLDER_PATH; }

		static std::string GetFolderFromPath(const std::string path)
		{
			size_t slashPos = path.find_last_of("/\\");
			return path.substr(0, slashPos + 1);
		}

		static std::string GetFilenameFromPath(const std::string path)
		{
			size_t slashPos = path.find_last_of("/\\");
			return path.substr(slashPos + 1);
		}

		static std::string GetFileExtension(const std::string path)
		{
			size_t dotPos = path.find_first_of(".");
			return path.substr(dotPos + 1);
		}
	};
}