//---------------------------------------------------------------
//
// ConsoleInputComponent.cpp
//

#include "ConsoleInputComponent.h"

#include "ICommand.h"
#include "IGameBoardViewStrategy.h"
#include "Game.h"
#include "UITextStrings.h"

#include <functional>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <sstream>

namespace Checkers {

//===============================================================

static constexpr unsigned char s_helpCommandId = 'h';
static constexpr unsigned char s_changeStyleCommandId = 's';
static constexpr unsigned char s_moveCommandId = 'm';

struct CommandResult {
	CommandResult(const CommandResult& other) = delete;
	CommandResult& operator=(const CommandResult& other) = delete;
	CommandResult(CommandResult&& other) noexcept = default;
	CommandResult& operator=(CommandResult&& other) noexcept = default;

	CommandResult(std::unique_ptr<ICommand> cmd, std::string&& invalidArgs = {})
		: command(std::move(cmd)), errorReason(std::move(invalidArgs)) {}
	~CommandResult() = default;

	// On successful command construction, this will be filled.
	std::unique_ptr<ICommand> command = nullptr;

	// If construction of a command fails, this message should have details as to why.
	std::string errorReason;
};

class CommandFactory {
public:
	CommandFactory(const CommandFactory& other) = delete;
	CommandFactory& operator=(const CommandFactory& other) = delete;
	CommandFactory(CommandFactory&& other) noexcept = default;
	CommandFactory& operator=(CommandFactory&& other) noexcept = default;

	CommandFactory()
	{
		// Register help command.
		m_commandRegistry[s_helpCommandId] =
			[](const std::vector<std::string>& args) -> CommandResult
		{
			return { std::make_unique<HelpCommand>() };
		};

		// Register the style change command.
		m_commandRegistry[s_changeStyleCommandId] =
			[](const std::vector<std::string>& args) -> CommandResult
		{
			// If we don't have the right number of arguments we can't even construct this command.
			if (args.size() != ChangeViewStyleCommand::Settings::s_requiredArgumentCount)
			{
				return { nullptr, UIText::s_errorCommandReasonInvalidArument.data()};
			}

			// We at least have the correct number of arguments, we can try and construct.
			return { std::make_unique<ChangeViewStyleCommand>(
				args[ChangeViewStyleCommand::Settings::s_viewStyleArgPosition]) };
		};

		// Register the move command.
		m_commandRegistry[s_moveCommandId] =
			[](const std::vector<std::string>& args) -> CommandResult
		{
			return { std::make_unique<MoveCommand>(
				args[MoveCommand::Settings::s_sourceArgPosition],
				args[MoveCommand::Settings::s_destinationArgPosition]) };
		};
	}

	~CommandFactory() = default;

	CommandResult CreateCommand(unsigned char cmdId, const std::vector<std::string>& args) const
	{
		// C++17 if-init https://en.cppreference.com/w/cpp/language/if
		if (const auto it = m_commandRegistry.find(cmdId); it != m_commandRegistry.end())
		{
			return it->second(args);
		}

		// Construction failed, errorInfo will be filled.
		return { nullptr, UIText::s_errorCommandReasonInvalidCommand.data() };
	}

private:

	// At the point where a command is needed, the appropriate delegate will be called to create the command.
	using CommandRegistrationDelegate = std::function<CommandResult(const std::vector<std::string>&)>;
	std::unordered_map<unsigned char, CommandRegistrationDelegate> m_commandRegistry;
};

ConsoleInputComponent::ConsoleInputComponent(Game* game)
	: m_game(game)
	, m_commandFactory(std::make_unique<CommandFactory>())
{
}
ConsoleInputComponent::~ConsoleInputComponent() = default;

//---------------------------------------------------------------

void ConsoleInputComponent::RequestAndProcessInput() const
{
	std::string input;
	std::getline(std::cin, input);

	if (input.empty())
	{
		return;
	}

	// The first character in the input is our identifier for which command to execute.
	const unsigned char commandId = input[0];

	// Skip over the commandId we just looked at.
	std::istringstream iss(input.substr(1));
	std::vector<std::string> args;

	std::string arg;
	while (iss >> arg)
	{
		args.push_back(arg);
	}

	const auto commandResult = m_commandFactory->CreateCommand(commandId, args);
	if (!commandResult.command)
	{
		spdlog::info("Failed to create command, it will not be executed. commandId={} input={}", commandId, input);
		ReportError(commandId, commandResult.errorReason);
	}
	else if (commandResult.command->IsCanceled())
	{
		spdlog::info("Skipped execution of canceled command. commandId={} input={}", commandId, input);
		ReportError(commandId, commandResult.command->GetErrorInfo().errorReason);
	}
	else
	{
		if (!commandResult.command->Execute(m_game))
		{
			spdlog::info("Failed to execute command. commandId={} input={}", commandId, input);
			ReportError(commandId, commandResult.command->GetErrorInfo().errorReason);
		}
	}
}

void ConsoleInputComponent::ReportError(unsigned char commandId, const std::string& reason) const
{
	m_game->GetUIPrompRequestedEvents().GetCommandErrorPromptRequestedEvent().notify(
		GetCommandInfoStringFromId(commandId, reason));
}

std::string ConsoleInputComponent::GetCommandInfoStringFromId(unsigned char commandId, const std::string& reason) const
{
	if (commandId == s_changeStyleCommandId)
	{
		std::string commandOptions;
		const auto& registeredViews =
			m_game->GetGameBoardViewStrategyRegistry()->GetRegisteredViews();

		// Starting at one since this is a player facing string.
		int32_t optionNumber = 1;
		for (const auto& view : registeredViews)
		{
			commandOptions += fmt::format("\n{}) {}\n", optionNumber++, view->GetId());
		}

		std::string fullInfoString = fmt::format(UIText::s_errorCommandChangeStyle,
			reason,
			commandOptions);

		return fullInfoString;
	}

	// Other commands NYI
	if (commandId == s_helpCommandId)
	{
		return "HELP_FAILURE_INFO_NYI";
	}
	if (commandId == s_moveCommandId)
	{
		return "MOVE_FAIL_INFO_NYI";
	}

	return UIText::s_errorCommandUnknown.data();
}

//===============================================================

}
