#pragma once

#include "IFileSystemBackend.h"

namespace Poly
{
	enum class EMountMode
	{
		Read,
		Write,
		ReadWrite
	};

	using MountHandle = uint32;

	class VirtualFileSystem
	{
	public:
		CLASS_STATIC(VirtualFileSystem);

		/*
		 * Mounts a file system backend to the virtual file system at the specified virtual root path.
		 * @param virtualRoot The virtual root path where the backend will be mounted.
		 * @param backend The file system backend to mount.
		 * @param mode The mount mode.
		 * @param priority The mount priority.
		 * @return The handle of the mounted file system backend.
		 */
		static MountHandle Mount(std::string_view virtualRoot, Unique<IFileSystemBackend> backend, EMountMode mode, int32 priority = 0);

		/*
		 * Unmounts a file system backend from the virtual file system.
		 * @param handle The handle of the mounted file system backend to unmount.
		 */
		static void Unmount(MountHandle handle);

		/*
		 * Checks if a file or directory exists in the virtual file system.
		 * @param virtualPath The virtual path to check.
		 */
		static bool Exists(std::string_view virtualPath);

		/*
		 * Checks if a virtual path is a directory in the virtual file system.
		 * @param virtualPath The virtual path to check.
		 */
		static bool IsDirectory(std::string_view virtualPath);

		/*
		 * Lists the files in a virtual directory in the virtual file system.
		 * Lists files from all mounted backends that match the virtual path, ensuring unique file names.
		 * @param virtualPath The virtual directory path to list files from.
		 */
		static std::vector<std::string> ListFiles(std::string_view virtualPath);

		/*
		 * Reads the contents of a file from the virtual file system, following priority of backends.
		 * @param virtualPath The virtual path of the file to read.
		 * @return A vector of bytes containing the file's contents, or an empty vector if the file does not exist or cannot be read.
		 */
		static std::vector<byte> Read(std::string_view virtualPath);

		/*
		 * Reads the contents of a text file from the virtual file system, following priority of backends.
		 * @param virtualPath The virtual path of the text file to read.
		 * @return A string containing the text file's contents, or an empty string if the file does not exist or cannot be read.
		 */
		static std::string ReadText(std::string_view virtualPath);

		/*
		 * Writes data to a file in the virtual file system, following priority of backends.
		 * @param virtualPath The virtual path of the file to write to.
		 * @param data The data to write to the file.
		 * @return True if the file was written successfully, false otherwise.
		 */
		static bool Write(std::string_view virtualPath, const std::vector<byte>& data);

		/*
		 * Writes text to a file in the virtual file system, following priority of backends.
		 * @param virtualPath The virtual path of the text file to write to.
		 * @param text The text to write to the file.
		 * @return True if the text file was written successfully, false otherwise.
		 */
		static bool WriteText(std::string_view virtualPath, std::string_view text);

		/*
		 * Resolves a virtual path to its physical path, following priority of backends.
		 * @param virtualPath The virtual path to resolve.
		 * @return The physical path if found, or an empty string if not found or cannot be resolved to a physical path.
		 */
		static std::string Resolve(std::string_view virtualPath);

	private:
		struct SMount
		{
			std::string                VirtualRoot;
			Unique<IFileSystemBackend> Backend;
			EMountMode                 Mode;
			int32                      Priority;
			uint32                     Handle;

			bool operator<(const SMount& other) const
			{
				return Priority < other.Priority;
			}
		};

		inline static std::vector<SMount> s_Mounts;
		inline static uint32              s_NextMountHandle = 0;
	};
} // namespace Poly