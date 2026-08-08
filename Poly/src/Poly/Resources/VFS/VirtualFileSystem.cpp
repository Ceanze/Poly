#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include "Poly/Resources/VFS/IFileSystemBackend.h"
#include "VirtualFileSystem.h"

#include <unordered_set>

namespace Poly
{
	MountHandle VirtualFileSystem::Mount(std::string_view virtualRoot, Unique<IFileSystemBackend> backend, EMountMode mode, int32 priority)
	{
		MountHandle handle = s_NextMountHandle++;
		SMount      mount  = {std::string(virtualRoot), std::move(backend), mode, priority, handle};
		s_Mounts.insert(std::upper_bound(s_Mounts.begin(), s_Mounts.end(), mount), std::move(mount));

		return handle;
	}

	void VirtualFileSystem::Unmount(MountHandle handle)
	{
		auto it = std::find_if(s_Mounts.begin(), s_Mounts.end(), [handle](const SMount& mount) { return mount.Handle == handle; });
		if (it != s_Mounts.end())
			s_Mounts.erase(it);
	}

	bool VirtualFileSystem::Exists(std::string_view virtualPath)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				return mount.Backend->Exists(relativePath);
			}
		}

		return false;
	}

	bool VirtualFileSystem::IsDirectory(std::string_view virtualPath)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				return mount.Backend->IsDirectory(relativePath);
			}
		}

		return false;
	}

	std::vector<std::string> VirtualFileSystem::ListFiles(std::string_view virtualPath)
	{
		std::unordered_set<std::string> uniqueFiles;
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot))
			{
				std::string              relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				std::vector<std::string> files        = mount.Backend->ListFiles(relativePath);
				uniqueFiles.insert(files.begin(), files.end());
			}
		}

		return std::vector<std::string>(uniqueFiles.begin(), uniqueFiles.end());
	}

	std::vector<byte> VirtualFileSystem::Read(std::string_view virtualPath)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot) && (mount.Mode == EMountMode::Read || mount.Mode == EMountMode::ReadWrite))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				if (mount.Backend->Exists(relativePath))
					return mount.Backend->Read(relativePath);
			}
		}

		return {};
	}

	std::string VirtualFileSystem::ReadText(std::string_view virtualPath)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot) && (mount.Mode == EMountMode::Read || mount.Mode == EMountMode::ReadWrite))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				if (mount.Backend->Exists(relativePath))
				{
					std::vector<byte> data = mount.Backend->Read(relativePath);
					return std::string(data.begin(), data.end());
				}
			}
		}

		return {};
	}

	bool VirtualFileSystem::Write(std::string_view virtualPath, const std::vector<byte>& data)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot) && (mount.Mode == EMountMode::Write || mount.Mode == EMountMode::ReadWrite))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				return mount.Backend->Write(relativePath, data);
			}
		}

		return false;
	}

	bool VirtualFileSystem::WriteText(std::string_view virtualPath, std::string_view text)
	{
		return false;
	}

	std::string VirtualFileSystem::Resolve(std::string_view virtualPath)
	{
		for (const auto& mount : s_Mounts)
		{
			if (virtualPath.starts_with(mount.VirtualRoot))
			{
				std::string relativePath = std::string(virtualPath.substr(mount.VirtualRoot.size()));
				auto        physicalPath = mount.Backend->ResolvePhysicalPath(relativePath);
				if (physicalPath.has_value())
					return physicalPath.value();
			}
		}

		return {};
	}
} // namespace Poly