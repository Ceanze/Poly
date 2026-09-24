#pragma once

namespace Poly
{
	namespace PathUtils
	{
		std::string GetFileName(std::string_view path);
		std::string GetDirectoryPath(std::string_view path);
		std::string GetExtension(std::string_view path);

		/**
		 * Strips the sub-asset fragment from a path, e.g. "models/foo.gltf#mesh_0" -> "models/foo.gltf".
		 * Paths without a fragment are returned unchanged
		 * @param path - path to strip, the returned view points into this string
		 * @return path of the source asset
		 */
		std::string_view GetSourcePath(std::string_view path);
	} // namespace PathUtils
} // namespace Poly