//---------------------------------------------------------------
//
// ICommand.cpp
//

#include "ICommand.h"

#include "Game.h"
#include "IGameBoardViewStrategy.h"
#include "UITextStrings.h"

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>

namespace Checkers {

//===============================================================

ICommand::~ICommand() = default;

//--------------------------------------------------------------

HelpCommand::~HelpCommand() = default;

bool HelpCommand::Execute(Game* game)
{
	// TODO: This command is kind of half-baked. Finish filling it out.

	// TODO: Let each command define a description and build the string that way.
	// Rather than hardcoding it in the json. If we change the name of a command we could
	// forget to update the strings in the json.
	game->GetUIEvents().GetHelpPromptRequestedEvent().notify();

	return true;
}

const CommandErrorInfo& HelpCommand::GetErrorInfo()
{
	// Help command can't error.
	static CommandErrorInfo info;
	return info;
}

//--------------------------------------------------------------

ChangeViewStyleCommand::~ChangeViewStyleCommand() = default;

ChangeViewStyleCommand::ChangeViewStyleCommand(const std::string& styleOption)
{
	int32_t optionValue = 0;
	const char* styleOptionBegin = styleOption.data();
	const char* styleOptionEnd = styleOption.data() + styleOption.size();

	// FYI: structured bindings: https://en.cppreference.com/w/cpp/language/structured_binding
	auto [inputAfterParse, ec] =

		// We use from_chars here because it avoids exception handling like std::stoi would need.
		std::from_chars(styleOptionBegin,
		styleOptionEnd,
		optionValue);

	// This just means there were trailing characters after the command was processed,
	// which can happen if the player fat fingers stuff.
	if (inputAfterParse != styleOptionEnd)
	{
		spdlog::warn("ChangeViewStyleCommand input may be malformed.inputAfterParse={}",
			inputAfterParse);
	}

	if(ec == std::errc())
	{
		m_optionValue = optionValue;
	}
	else
	{
		// We cannot complete this request, let's fill out why.
		m_isCanceled = true;

		if (ec == std::errc::invalid_argument)
		{
			m_errorInfo.errorReason = UIText::s_errorCommandReasonInvalidArument;
		}
		else
		{
			spdlog::warn("Command canceled for unknown reason.");
			m_errorInfo.errorReason = UIText::s_errorCommandReasonUnknown;
		}
	}
}

bool ChangeViewStyleCommand::Execute(Game* game)
{
	// The player selected an option from the list of registered views. So we need to lookup
	// which view that was.
	const GameBoardViewStrategyRegistry* viewRegistry = game->GetGameBoardViewStrategyRegistry();
	const IGameBoardViewStrategy* newViewStrategy = viewRegistry->GetGameBoardViewStrategyForPlayerOption(m_optionValue);

	// The player selected a view that doesn't exist.
	if (!newViewStrategy)
	{
		m_errorInfo.errorReason = UIText::s_errorCommandChangeStyleReasonNotFound;
		return false;
	}

	// The player tried to select a view that is already the active one.
	if (newViewStrategy->GetId() == game->GetSelectedGameBoardViewStrategy()->GetId())
	{
		m_errorInfo.errorReason = UIText::s_errorCommandChangeStyleReasonAlreadySelected;
		return false;
	}

	// We should have handled all possible errors by this point.
	game->SetSelectedGameBoardViewStrategy(newViewStrategy->GetId());
	return true;
}

void ChangeViewStyleCommand::Cancel()
{
	m_isCanceled = true;
}

bool ChangeViewStyleCommand::IsCanceled() const
{
	return m_isCanceled;
}

const CommandErrorInfo& ChangeViewStyleCommand::GetErrorInfo()
{
	return m_errorInfo;
}

//--------------------------------------------------------------

MoveCommand::~MoveCommand() = default;

MoveCommand::MoveCommand(const std::string& sourceInput, const std::string& destinationInput)
	: m_sourceInput(sourceInput)
	, m_destinationInput(destinationInput)
{
	// We let the view validate whether an argument is well-formed because its format is
	// tightly coupled to its display to the player. Attempting to do that here would impose
	// restrictions on what kinds of arguments a view can accept, and therefore how it can
	// display to the player.
}

bool MoveCommand::Execute(Game* game)
{
	// NYI
	int32_t sourceIndex = game->GetSelectedGameBoardViewStrategy()->GetGameBoardIndexFromInput(m_sourceInput);
	int32_t destinationIndex = game->GetSelectedGameBoardViewStrategy()->GetGameBoardIndexFromInput(m_destinationInput);

	if (sourceIndex == GameBoardStatics::s_invalidBoardIndex ||
		destinationIndex == GameBoardStatics::s_invalidBoardIndex)
	{
		m_errorInfo.errorReason = UIText::s_errorCommandMoveReasonInvalidMoveDescription;
		return false;
	}

	PieceMoveDescription pieceMoveDescription{ sourceIndex, destinationIndex };
	spdlog::info("Player wants to make a move. MoveDescription={}", pieceMoveDescription);

	game->MovePiece(pieceMoveDescription);

	return true;
}

void MoveCommand::Cancel()
{
	m_isCanceled = true;
}

const CommandErrorInfo& MoveCommand::GetErrorInfo()
{
	return m_errorInfo;
}

//===============================================================

}
