#pragma once

#include "IFileSystemBackend.h"

#include <filesystem>

namespace Poly
{
	class FileDirectoryBackend : public IFileSystemBackend
	{
	public:
		FileDirectoryBackend(std::string_view physicalPath);
		virtual ~FileDirectoryBackend() = default;

		bool                     Exists(std::string_view relativePath) const override;
		bool                     IsDirectory(std::string_view relativePath) const override;
		std::vector<std::string> ListFiles(std::string_view relativePath) const override;

		std::vector<byte> Read(std::string_view relativePath) const override;
		bool              Write(std::string_view relativePath, const std::vector<byte>& data) override;

		std::optional<std::string> ResolvePhysicalPath(std::string_view relativePath) const override;

	private:
		std::filesystem::path m_PhysicalPath;
	};
} // namespace Poly