#pragma once

namespace Poly
{
	namespace PathUtils
	{
		std::string GetFileName(std::string_view path);
		std::string GetDirectoryPath(std::string_view path);
		std::string GetExtension(std::string_view path);
	} // namespace PathUtils
} // namespace Poly