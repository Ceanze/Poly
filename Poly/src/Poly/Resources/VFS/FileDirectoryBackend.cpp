#include "FileDirectoryBackend.h"

#include <fstream>

namespace Poly
{
	FileDirectoryBackend::FileDirectoryBackend(std::string_view physicalPath)
	    : m_PhysicalPath(physicalPath)
	{}

	bool FileDirectoryBackend::Exists(std::string_view relativePath) const
	{
		return std::filesystem::exists(m_PhysicalPath / relativePath);
	}

	bool FileDirectoryBackend::IsDirectory(std::string_view relativePath) const
	{
		return std::filesystem::is_directory(m_PhysicalPath / relativePath);
	}

	std::vector<std::string> FileDirectoryBackend::ListFiles(std::string_view relativePath) const
	{
		if (!IsDirectory(relativePath))
			return {};

		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(m_PhysicalPath / relativePath))
		{
			files.push_back(entry.path().filename().string());
		}

		return files;
	}

	std::vector<byte> FileDirectoryBackend::Read(std::string_view relativePath) const
	{
		std::ifstream file(m_PhysicalPath / relativePath, std::ios::binary);
		if (!file)
			return {};

		file.seekg(0, std::ios::end);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<byte> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
			return {};

		return buffer;
	}

	bool FileDirectoryBackend::Write(std::string_view relativePath, const std::vector<byte>& data)
	{
		std::ofstream file(m_PhysicalPath / relativePath, std::ios::binary);
		if (!file)
			return false;

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		return file.good();
	}

	std::optional<std::string> FileDirectoryBackend::ResolvePhysicalPath(std::string_view relativePath) const
	{
		if (Exists(relativePath))
		{
			return (m_PhysicalPath / relativePath).string();
		}

		return std::nullopt;
	}

} // namespace Poly