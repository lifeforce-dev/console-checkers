//---------------------------------------------------------------
//
// ConsoleGameDisplay.h
//

#pragma once

#include <string>

namespace Checkers {

//===============================================================

class UIPromptRequestedEvents;
class IGameStateDisplayInfo;

class ConsoleGameDisplay
{
public:
	ConsoleGameDisplay();
	~ConsoleGameDisplay() = default;

	// Sets up our display and events for rendering and usage. Must be called before anything else!
	void Initialize(IGameStateDisplayInfo* displayData, UIPromptRequestedEvents& events);

private:
	// Builds and returns the visual representation of the game board according to game state.
	std::string GetGameBoardDisplay() const;

private:
	// Our interface into looking at the game data.
	IGameStateDisplayInfo* m_gameStateInfo = nullptr;

	// We will respond to these events by showing the player what they need to know.
	UIPromptRequestedEvents* m_events = nullptr;

	// Console display is fully hooked up and ready to use.
	bool m_isInitialized = false;
};

//===============================================================

}
