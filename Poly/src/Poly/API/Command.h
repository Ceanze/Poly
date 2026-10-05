#pragma once

#include "Poly/Core/Result.h"

namespace Poly::API
{
	class EngineContext;

	/*
	 * A single undoable change to the world. All changes made through the API are commands run by the CommandHistory.
	 *
	 * Rules for a command:
	 * - Entities are referred to by PolyID, never by Entity - an entity that is destroyed and restored by an undo is
	 *   a new Entity with the same PolyID
	 * - Execute() validates everything before changing anything, a failed Execute() leaves the world untouched
	 * - Execute() gathers whatever Undo() needs, and can be called again after an Undo() (redo)
	 * - The events describing the change are queued by both Execute() and Undo()
	 */
	class Command
	{
	public:
		virtual ~Command() = default;

		virtual Result<void> Execute(EngineContext& context) = 0;
		virtual void         Undo(EngineContext& context)    = 0;

		/**
		 * @return name of the command for display, e.g. in an undo history
		 */
		virtual std::string_view GetName() const = 0;

		/**
		 * Tries to absorb a command that was executed right after this one, see CommandHistory::BeginMerge()
		 * @param next - the already executed command
		 * @return true if this command now also covers the change of next, next is then dropped
		 */
		virtual bool TryMerge(const Command& next) { return false; }
	};
} // namespace Poly::API
