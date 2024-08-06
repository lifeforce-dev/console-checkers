//---------------------------------------------------------------
//
// ConsoleGameDisplay.h
//

#pragma once

#include <string>

namespace Checkers {

//===============================================================

class IGameBoardViewStrategy;
class UIEvents;
class IGameStateDisplayInfo;

class ConsoleGameDisplay
{
public:
	// Sets up our display and events for rendering and usage. Must be called before anything else!
	void Initialize(IGameStateDisplayInfo* displayData, UIEvents& events);

private:
	// Builds and returns the visual representation of the game board according to game state.
	std::string GetGameBoardDisplay() const;

	// Returns the whole game screen. Use this when redrawing the game state each turn.
	std::string GetFullGameDisplayText() const;

private:
	// Our interface into looking at the game data.
	IGameStateDisplayInfo* m_gameStateInfo = nullptr;

	// Handles the visual representation of our game board.
	IGameBoardViewStrategy* m_gameBoardView = nullptr;

	// We will respond to these events by showing the player what they need to know.
	UIEvents* m_events = nullptr;

	// Console display is fully hooked up and ready to use.
	bool m_isInitialized = false;
};

//===============================================================

}
