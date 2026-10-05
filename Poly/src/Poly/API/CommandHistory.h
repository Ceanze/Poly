#pragma once

#include "Poly/Core/Result.h"

namespace Poly::API
{
	class EngineContext;
	class ICommand;

	/*
	 * Runs commands and keeps them for undo and redo.
	 *
	 * Usage:
	 * @code
	 * history.Execute(CreateUnique<SetFieldCommand>(id, "Transform", "Scale", "[2,2,2]"));
	 * history.Undo();
	 * history.Redo();
	 * @endcode
	 */
	class CommandHistory
	{
	public:
		explicit CommandHistory(EngineContext& context);
		~CommandHistory() = default;
		CLASS_REMOVE_COPY(CommandHistory);

		/**
		 * Executes a command, and keeps it for undo if it succeeded. Discards everything that could be redone
		 * @return the result of the command
		 */
		Result<void> Execute(Unique<Command> pCommand);

		Result<void> Undo();
		Result<void> Redo();

		bool CanUndo() const;
		bool CanRedo() const;

		/**
		 * Starts merging the commands that follow into a single undo step where possible (Command::TryMerge()), until
		 * EndMerge(). Used for continuous edits like dragging a value, where every frame executes a command
		 */
		void BeginMerge();
		void EndMerge();

		/**
		 * @return true if the world has changed since the last MarkSaved() or Clear()
		 */
		bool IsDirty() const;

		/**
		 * Marks the current state as the saved one
		 */
		void MarkSaved();

		/**
		 * Forgets all commands and marks the current state as the saved one, used when the world is replaced
		 */
		void Clear();

	private:
		static constexpr size_t NEVER_SAVED = ~size_t(0);

		void QueueChangedEvent();

		EngineContext& m_Context;

		std::vector<Unique<Command>> m_UndoStack;
		std::vector<Unique<Command>> m_RedoStack;

		size_t m_SavedUndoSize = 0;

		bool m_Merging        = false;
		bool m_HasMergeTarget = false;
	};
} // namespace Poly::API
