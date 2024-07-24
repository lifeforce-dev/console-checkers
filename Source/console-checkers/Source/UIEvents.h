//---------------------------------------------------------------
//
// UIEvents.h
//

#pragma once

#include <observable/observable.hpp>


namespace Checkers {

//===============================================================

class IGameBoardViewStrategy;

// These events are designed to provide a way for the game to indicate that user
// notification or input is needed.
class UIEvents
{
public:

	// The game is requesting that we show a welcome prompt to the player.
	using WelcomePromptRequestedEvent = observable::subject<void()>;
	WelcomePromptRequestedEvent& GetWelcomePromptRequestedEvent() { return m_welcomePromptRequestedEvent; }

	// The game has requested that we change the visual representation of the checkers board.
	using GameBoardViewStrategyChangedEvent = observable::subject<void(IGameBoardViewStrategy*)>;
	GameBoardViewStrategyChangedEvent& GetGameBoardViewStrategyChangedEvent() { return m_gameBoardViewStrategyChangedEvent; }

private:
	WelcomePromptRequestedEvent m_welcomePromptRequestedEvent;
	GameBoardViewStrategyChangedEvent m_gameBoardViewStrategyChangedEvent;
};

//===============================================================

}
