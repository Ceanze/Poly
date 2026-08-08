#pragma once

#include <optional>
#include <string_view>

namespace Poly
{
	class IFileSystemBackend
	{
	public:
		virtual ~IFileSystemBackend() = default;

		virtual bool                     Exists(std::string_view relativePath) const      = 0;
		virtual bool                     IsDirectory(std::string_view relativePath) const = 0;
		virtual std::vector<std::string> ListFiles(std::string_view relativePath) const   = 0;

		virtual std::vector<byte> Read(std::string_view relativePath) const                           = 0;
		virtual bool              Write(std::string_view relativePath, const std::vector<byte>& data) = 0;

		virtual std::optional<std::string> ResolvePhysicalPath(std::string_view relativePath) const = 0;
	};
} // namespace Poly