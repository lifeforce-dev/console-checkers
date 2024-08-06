//---------------------------------------------------------------
//
// ConsoleInputComponent.h
//

#pragma once

#include <memory>
#include <string>

namespace Checkers {

//===============================================================

class CommandFactory;
class Game;
class ICommand;
class ConsoleInputComponent
{
public:
	ConsoleInputComponent(const ConsoleInputComponent& other) = delete;
	ConsoleInputComponent& operator=(const ConsoleInputComponent& other) = delete;

	ConsoleInputComponent(ConsoleInputComponent&& other) = default;
	ConsoleInputComponent& operator=(ConsoleInputComponent&& other) = default;

	ConsoleInputComponent(Game* game);
	~ConsoleInputComponent();

	// Waits for input, then parses and constructs the appropriate command objects, and executes them.
	void RequestAndProcessInput() const;

private:

	// Reports a comprehensive error to the UI.
	void ReportError(unsigned char commandId, const std::string& reason) const;

	// On failure, contextualizes a failure and reports to the UI who notifies the player.
	std::string GetCommandInfoStringFromId(unsigned char commandId, const std::string& reason) const;

	// Ptr to our parent. It is acceptable for our commands to interact with Game.
	Game* m_game = nullptr;

	// Registers and creates commands for us.
	std::unique_ptr<CommandFactory> m_commandFactory = nullptr;
};

//===============================================================

}
