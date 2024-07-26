//---------------------------------------------------------------
//
// ICommand.cpp
//

#include "ICommand.h"

#include "Game.h"
#include "IGameBoardViewStrategy.h"

#include <spdlog/fmt/bundled/format.h>
#include <spdlog/spdlog.h>

namespace Checkers {

//===============================================================

ICommand::~ICommand() = default;

//--------------------------------------------------------------

HelpCommand::~HelpCommand() = default;

void HelpCommand::Execute(Game* game)
{
	// TODO: Let each command define a description and build the string that way.
	// Rather than hardcoding it in the json. If we change the name of a command we could
	// forget to update the strings in the json.
	game->GetUIPrompRequestedEvents().GetHelpPromptRequestedEvent().notify();
}

//--------------------------------------------------------------

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

	// This just means there were trailing characters after the command was processed.
	if (inputAfterParse != styleOptionEnd)
	{
		spdlog::warn("ChangeViewStyleCommand input may be malformed.inputAfterParse={}",
			inputAfterParse);
	}

	if (ec == std::errc())
	{
		const bool isValidViewStrategy = optionValue > static_cast<int32_t>(GameBoardViewStrategyId::Begin) &&
			optionValue < static_cast<int32_t>(GameBoardViewStrategyId::End);

		// We need to figure out what to do in the case of an invalid strategy.
		m_newViewStrategy = isValidViewStrategy ?
			static_cast<GameBoardViewStrategyId>(optionValue) : GameBoardViewStrategyId::End;
	}
	else if (ec == std::errc::invalid_argument)
	{
		// TODO: handle this error.
	}

}

ChangeViewStyleCommand::~ChangeViewStyleCommand() = default;

void ChangeViewStyleCommand::Execute(Game* game)
{
	// TODO: I don't know if we should need this check here. Cancellation should take care of this.
	if (m_newViewStrategy != GameBoardViewStrategyId::End)
	{
		game->SetSelectedGameBoardViewStrategy(m_newViewStrategy);
	}
}

void ChangeViewStyleCommand::Cancel()
{
	m_isCanceled = true;
}

bool ChangeViewStyleCommand::IsCanceled()
{
	// NYI
	return false;
}

//--------------------------------------------------------------

MoveCommand::~MoveCommand() = default;

MoveCommand::MoveCommand(const std::string& sourceInput, const std::string& destinationInput)
{
}

void MoveCommand::Execute(Game* game)
{
	// NYI
	int32_t sourceIndex = game->GetSelectedGameBoardViewStrategy()->GameBoardPositionToIndex(m_sourceInput);
	int32_t destinationIndex = game->GetSelectedGameBoardViewStrategy()->GameBoardPositionToIndex(m_destinationInput);

	// TODO: fire event for the move passing the indices that we need to act upon.

}

void MoveCommand::Cancel()
{
	m_isCanceled = true;
}

//===============================================================

}
