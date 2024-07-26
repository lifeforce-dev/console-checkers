//---------------------------------------------------------------
//
// ConsoleInputComponent.cpp
//

#include "ConsoleInputComponent.h"

#include "ICommand.h"

#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace Checkers {

//===============================================================

class CommandFactory {
public:
	CommandFactory(const CommandFactory& other) = delete;
	CommandFactory& operator=(const CommandFactory& other) = delete;
	CommandFactory(CommandFactory&& other) noexcept = default;
	CommandFactory& operator=(CommandFactory&& other) noexcept = default;

	CommandFactory()
	{
		// TODO: Add arg support for hints. When the player asks for a hint, the background color
		// of available moves will change.
		m_commandRegistry['h'] = [](const std::vector<std::string> args)
		{
			return std::make_unique<HelpCommand>();
		};
		m_commandRegistry['s'] = [](const std::vector<std::string> args)
		{

			return std::make_unique<ChangeViewStyleCommand>(
				args[ChangeViewStyleCommand::Settings::s_viewStyleArgPosition]);
		};
		m_commandRegistry['m'] = [](const std::vector<std::string> args)
		{
			return std::make_unique<MoveCommand>(args[MoveCommand::Settings::s_sourceArgPosition],
				args[MoveCommand::Settings::s_destinationArgPosition]);
		};
	}

	~CommandFactory() = default;

	std::unique_ptr<ICommand> CreateCommand(unsigned char cmdId, const std::vector<std::string>& args) const
	{
		auto it = m_commandRegistry.find(cmdId);
		if (it != m_commandRegistry.end())
		{
			return it->second(args);
		}

		return nullptr;
	}

private:
	using CommandRegistrationDelegate = std::function<std::unique_ptr<ICommand>(const std::vector<std::string>)>;
	std::unordered_map<unsigned char, CommandRegistrationDelegate> m_commandRegistry;
};

ConsoleInputComponent::ConsoleInputComponent(Game* game)
	: m_game(game)
	, m_commandFactory(std::make_unique<CommandFactory>())
{
	
}
ConsoleInputComponent::~ConsoleInputComponent() = default;

//---------------------------------------------------------------

void ConsoleInputComponent::RequestAndProcessInput()
{
	std::string input;
	std::getline(std::cin, input);

	if (input.empty())
	{
		return;
	}

	// TODO: Should I trim leading and trailing whitespace?

	// The first character in the input is our identifier for which command to execute.
	const char commandId = input[0];

	// Skip over the command we just looked at.
	std::istringstream iss(input.substr(1));
	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg)
	{
		args.push_back(arg);
	}

	const auto command = m_commandFactory->CreateCommand(commandId, args);
	if (!command)
	{
		// TODO: error handling
		return;
	}
	if (!command->IsCanceled())
	{
		command->Execute(m_game);
	}
	else
	{
		spdlog::info("Skipped execution of canceled command. commandId={}", commandId);
	}

	// TODO:
	// if command doesn't exist
		// notify UI of the error


}

//===============================================================

}
