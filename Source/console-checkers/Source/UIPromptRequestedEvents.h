//---------------------------------------------------------------
//
// UIPromptRequestedEvents.h
//

#pragma once

#include <observable/observable.hpp>

namespace Checkers {

//===============================================================

// These events are designed to provide a way for the game to indicate that user
// notification or input is needed.
class UIPromptRequestedEvents
{
public:

	// The game is requesting that we show a welcome prompt to the player.
	using WelcomePromptRequestedEvent = observable::subject<void()>;
	WelcomePromptRequestedEvent& GetWelcomePromptRequestedEvent() { return m_welcomePromptRequestedEvent; }

private:
	WelcomePromptRequestedEvent m_welcomePromptRequestedEvent;
};

//===============================================================

}
