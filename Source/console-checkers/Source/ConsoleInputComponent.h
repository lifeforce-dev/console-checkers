//---------------------------------------------------------------
//
// ConsoleInputComponent.h
//

#pragma once

#include <memory>

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

	void RequestAndProcessInput();

private:

	// Ptr to our parent. It is acceptable for our commands to interact with Game.
	Game* m_game = nullptr;

	// Registers and creates commands for us.
	std::unique_ptr<CommandFactory> m_commandFactory = nullptr;
};

//===============================================================

}
