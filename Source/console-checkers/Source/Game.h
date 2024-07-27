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

class ConsoleGameDisplay;
class ConsoleInputComponent;
class GameState;
class IGameBoardViewStrategy;
class PlayerState;

class GameBoardViewStrategyRegistry
{
public:
	GameBoardViewStrategyRegistry(const GameBoardViewStrategyRegistry& other) = delete;
	GameBoardViewStrategyRegistry& operator=(const GameBoardViewStrategyRegistry& other) = delete;
	GameBoardViewStrategyRegistry(GameBoardViewStrategyRegistry&& other) noexcept = default;
	GameBoardViewStrategyRegistry& operator=(GameBoardViewStrategyRegistry&& other) noexcept = default;

	GameBoardViewStrategyRegistry();
	~GameBoardViewStrategyRegistry() = default;

	// Take a view Id and checks if it is registered at all.
	bool IsRegisteredView(GameBoardViewStrategyId id) const;

	// Returns the list of all registered views.
	const std::vector<std::unique_ptr<IGameBoardViewStrategy>>& GetRegisteredViews() const;

	// Given an id will return the associated game board strategy.
	IGameBoardViewStrategy* GetGameBoardViewStrategyForId(GameBoardViewStrategyId id) const;

	// Given a player-facing option (1 based), get the corresponding view.
	IGameBoardViewStrategy* GetGameBoardViewStrategyForPlayerOption(int32_t option) const;

private:

	// We could use a std::unordered_map here, but the order that they're registered is how
	// they will appear in the UI, and how the player will select one.
	std::vector<std::unique_ptr<IGameBoardViewStrategy>> m_registeredGameBoardViews;
};

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
	GameBoardViewStrategyRegistry* GetGameBoardViewStrategyRegistry() const { return m_gameBoardViewStrategyRegistry.get(); }

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
