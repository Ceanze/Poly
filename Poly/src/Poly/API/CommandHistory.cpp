#include "CommandHistory.h"

#include "Poly/API/Command.h"
#include "Poly/API/EngineContext.h"

namespace Poly::API
{
	CommandHistory::CommandHistory(EngineContext& context)
	    : m_Context(context)
	{}

	Result<void> CommandHistory::Execute(Unique<Command> pCommand)
	{
		if (Result<void> result = pCommand->Execute(m_Context); !result)
			return result;

		// The saved state was among the commands that can no longer be redone
		if (m_SavedUndoSize != NEVER_SAVED && m_SavedUndoSize > m_UndoStack.size())
			m_SavedUndoSize = NEVER_SAVED;
		m_RedoStack.clear();

		if (m_Merging && m_HasMergeTarget && m_UndoStack.back()->TryMerge(*pCommand))
		{
			// The top command changed, so the state it was saved in is gone
			if (m_SavedUndoSize == m_UndoStack.size())
				m_SavedUndoSize = NEVER_SAVED;
		}
		else
		{
			m_UndoStack.push_back(std::move(pCommand));
			m_HasMergeTarget = m_Merging;
		}

		QueueChangedEvent();
		return {};
	}

	Result<void> CommandHistory::Undo()
	{
		if (m_UndoStack.empty())
			return MakeError(ErrorCode::NothingToUndo, "There is nothing to undo");

		Unique<Command> pCommand = std::move(m_UndoStack.back());
		m_UndoStack.pop_back();

		pCommand->Undo(m_Context);
		m_RedoStack.push_back(std::move(pCommand));
		m_HasMergeTarget = false;

		QueueChangedEvent();
		return {};
	}

	Result<void> CommandHistory::Redo()
	{
		if (m_RedoStack.empty())
			return MakeError(ErrorCode::NothingToRedo, "There is nothing to redo");

		// Kept on the redo stack if it fails, the world is then unchanged
		if (Result<void> result = m_RedoStack.back()->Execute(m_Context); !result)
			return result;

		m_UndoStack.push_back(std::move(m_RedoStack.back()));
		m_RedoStack.pop_back();
		m_HasMergeTarget = false;

		QueueChangedEvent();
		return {};
	}

	bool CommandHistory::CanUndo() const
	{
		return !m_UndoStack.empty();
	}

	bool CommandHistory::CanRedo() const
	{
		return !m_RedoStack.empty();
	}

	void CommandHistory::BeginMerge()
	{
		m_Merging        = true;
		m_HasMergeTarget = false;
	}

	void CommandHistory::EndMerge()
	{
		m_Merging        = false;
		m_HasMergeTarget = false;
	}

	bool CommandHistory::IsDirty() const
	{
		return m_SavedUndoSize != m_UndoStack.size();
	}

	void CommandHistory::MarkSaved()
	{
		m_SavedUndoSize  = m_UndoStack.size();
		m_HasMergeTarget = false;
		QueueChangedEvent();
	}

	void CommandHistory::Clear()
	{
		m_UndoStack.clear();
		m_RedoStack.clear();
		m_SavedUndoSize  = 0;
		m_HasMergeTarget = false;
		QueueChangedEvent();
	}

	void CommandHistory::QueueChangedEvent()
	{
		m_Context.Events().Queue(HistoryChanged{.CanUndo = CanUndo(), .CanRedo = CanRedo(), .Dirty = IsDirty()});
	}
} // namespace Poly::API
