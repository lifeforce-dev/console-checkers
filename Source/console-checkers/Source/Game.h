//---------------------------------------------------------------
//
// Game.h
//

#pragma once

#include "GameTypes.h"
#include "UIEvents.h"

#include <memory>
#include <vector>

namespace Checkers {

//===============================================================

class GameState;
class ConsoleGameDisplay;
class ConsoleInputComponent;
class GameBoardViewStrategyRegistry;
class IGameBoardViewStrategy;
class PlayerState;

class Game
{
public:
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;

	Game(Game&& other) noexcept = default;
	Game& operator=(Game&& other) noexcept = default;

	Game();
	~Game();

	// Getters
	IGameBoardViewStrategy* GetSelectedGameBoardViewStrategy() const { return m_selectedGameBoardViewStrategy; }
	GameState* GetGameState() const { return m_gameState.get(); }
	UIEvents& GetUIPrompRequestedEvents() { return m_uiPromptRequestedEvents; }

	// Setters
	void SetSelectedGameBoardViewStrategy(GameBoardViewStrategyId id);

	// Runs the game until a win condition is met.
	void Run();

private:

	// InputComponent handles input and delegates commands to where they need to go.
	std::unique_ptr<ConsoleInputComponent> m_inputComponent = nullptr;

	// Holds the entire state of the game, including the board, player turn, etc.
	std::unique_ptr<GameState> m_gameState = nullptr;

	// Each player has a state object with info specific to that player. This list holds each player.
	std::vector<std::unique_ptr<PlayerState>> m_playerStates;

	// Registry of our available view strategies.
	std::unique_ptr<GameBoardViewStrategyRegistry> m_gameBoardViewStrategyRegistry = nullptr;

	// Events specifically for the Display to listen for.
	UIEvents m_uiPromptRequestedEvents;

	// Determines how the game data is displayed to the user, and translates move string formats to game input.
	IGameBoardViewStrategy* m_selectedGameBoardViewStrategy = nullptr;
};

//===============================================================

}
