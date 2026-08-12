#pragma once

#include <functional>
#include <optional>
#include <string_view>

namespace Poly
{
	struct FileSystemEntry
	{
		std::string RelativePath;
		bool        IsDirectory;
		uint64      Size = 0;
	};

	class IFileSystemBackend
	{
	public:
		virtual ~IFileSystemBackend() = default;

		virtual bool                     Exists(std::string_view relativePath) const      = 0;
		virtual bool                     IsDirectory(std::string_view relativePath) const = 0;
		virtual std::vector<std::string> ListFiles(std::string_view relativePath) const   = 0;

		/**
		 * Recursively enumerates all entries under relativePath ("" = backend root).
		 */
		virtual void EnumerateFiles(std::string_view relativePath, const std::function<void(const FileSystemEntry&)>& callback) const = 0;

		virtual std::vector<byte> Read(std::string_view relativePath) const                           = 0;
		virtual bool              Write(std::string_view relativePath, const std::vector<byte>& data) = 0;

		virtual std::optional<std::string> ResolvePhysicalPath(std::string_view relativePath) const = 0;
	};
} // namespace Poly