#pragma once

#include "Poly/Core/Result.h"

namespace Poly::API
{
	class EngineContext;

	/*
	 * Creating, loading and saving the world of the context.
	 *
	 * New() and Load() replace all entities and cannot be undone, they clear the undo history instead.
	 * The systems of the world are kept.
	 */
	class WorldService
	{
	public:
		explicit WorldService(EngineContext& context);

		/**
		 * Destroys all entities and renames the world
		 * @param name - name of the new world
		 */
		void New(std::string_view name = "Untitled");

		/**
		 * Replaces the entities of the world with the ones of a file
		 * @param vfsPath - VFS path of the .polyworld file
		 * @return error if the file could not be read or is invalid, the world is then left untouched
		 */
		Result<void> Load(std::string_view vfsPath);

		/**
		 * Saves the world
		 * @param vfsPath - VFS path of the file to write, empty to use the path the world was loaded from or last saved to
		 */
		Result<void> Save(std::string_view vfsPath = "");

		const std::string& GetName() const;

		/**
		 * @return VFS path the world was loaded from or last saved to, empty if neither has happened
		 */
		const std::string& GetPath() const { return m_Path; }

		/**
		 * @return true if the world has changed since it was created, loaded or last saved
		 */
		bool IsDirty() const;

	private:
		EngineContext& m_Context;
		std::string    m_Path;
	};
} // namespace Poly::API
