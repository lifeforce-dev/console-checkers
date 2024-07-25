//---------------------------------------------------------------
//
// Game.h
//

#pragma once

#include "UIEvents.h"

#include <memory>
#include <vector>

namespace Checkers {

//===============================================================

class GameState;
class CheckersNotationView;
class ChessLikeView;
class ConsoleGameDisplay;
class ConsoleInputComponent;
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
	GameState* GetGameState() const { return m_gameState.get(); }
	UIEvents& GetUIPrompRequestedEvents() { return m_uiPromptRequestedEvents; }

	// Runs the game until a win condition is met.
	void Run();

private:

	// InputComponent handles input and delegates commands to where they need to go.
	std::unique_ptr<ConsoleInputComponent> m_inputComponent = nullptr;

	// Holds the entire state of the game, including the board, player turn, etc.
	std::unique_ptr<GameState> m_gameState = nullptr;

	// Each player has a state object with info specific to that player. This list holds each player.
	std::vector<std::unique_ptr<PlayerState>> m_playerStates;

	// Events specifically for the Display to listen for.
	UIEvents m_uiPromptRequestedEvents;

	// Game board views. Changes how the board is represented, and can be changed by the player.
	std::unique_ptr<ChessLikeView> m_chessLikeGameView = nullptr;
	std::unique_ptr<CheckersNotationView> m_checkersNotationView = nullptr;
};

//===============================================================

}
