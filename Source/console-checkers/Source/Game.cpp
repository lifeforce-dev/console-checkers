//---------------------------------------------------------------
//
// Game.h
//

#include "Game.h"

#include "CheckersNotationView.h"
#include "ChessLikeView.h"
#include "ConsoleInputComponent.h"
#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "PlayerState.h"

#include <spdlog/spdlog.h>

// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>

namespace Checkers {

//===============================================================

class GameBoardViewStrategyRegistry
{
public:
	GameBoardViewStrategyRegistry(const GameBoardViewStrategyRegistry& other) = delete;
	GameBoardViewStrategyRegistry& operator=(const GameBoardViewStrategyRegistry& other) = delete;
	GameBoardViewStrategyRegistry(GameBoardViewStrategyRegistry&& other) noexcept = default;
	GameBoardViewStrategyRegistry& operator=(GameBoardViewStrategyRegistry&& other) noexcept = default;

	GameBoardViewStrategyRegistry()
	{
		m_registeredGameBoardViews[GameBoardViewStrategyId::CheckersNotation] =
			std::make_unique<CheckersNotationView>();
		m_registeredGameBoardViews[GameBoardViewStrategyId::ChessLikeView] =
			std::make_unique<ChessLikeView>();
	}

	~GameBoardViewStrategyRegistry() = default;

	// Given an id will return the associated game board strategy.
	IGameBoardViewStrategy* GetGameBoardViewStrategy(GameBoardViewStrategyId id)
	{
		auto it = m_registeredGameBoardViews.find(id);
		if (it != m_registeredGameBoardViews.end())
		{
			return it->second.get();
		}

#ifdef DEBUG
		// Hi! If you're here, you forgot to add the view to the registry :).
		assert(true);
#endif

		spdlog::error("Attempted to use unregistered game view strategy. id={}", id);
		return nullptr;
	}

private:
	std::unordered_map<GameBoardViewStrategyId,
		std::unique_ptr<IGameBoardViewStrategy>> m_registeredGameBoardViews;
};

Game::Game()
	: m_inputComponent(std::make_unique<ConsoleInputComponent>(this))
	, m_gameState(std::make_unique<GameState>())
	, m_gameBoardViewStrategyRegistry(std::make_unique<GameBoardViewStrategyRegistry>())
{
	// Creates both players.
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Red));
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Black));

	m_selectedGameBoardViewStrategy = m_gameBoardViewStrategyRegistry->GetGameBoardViewStrategy(
		GameplaySettings::s_defaultGameBoardViewStrategy);

	// TODO: Its not obvious that we CANNOT fire off UI events in here. Display
	// is not alive yet. But we don't know that. Our owner should probably tell us.
	// Or Display could via UI events but that seems a bit out of place.
}

Game::~Game() = default;

void Game::SetSelectedGameBoardViewStrategy(GameBoardViewStrategyId id)
{
	m_selectedGameBoardViewStrategy = m_gameBoardViewStrategyRegistry->GetGameBoardViewStrategy(id);

	// Our UI needs to be notified of the change.
	m_uiPromptRequestedEvents.GetGameBoardViewStrategyChangedEvent().notify(m_selectedGameBoardViewStrategy);
}

void Game::Run()
{
	m_uiPromptRequestedEvents.GetGameBoardViewStrategyChangedEvent().notify(m_selectedGameBoardViewStrategy);
	m_uiPromptRequestedEvents.GetWelcomePromptRequestedEvent().notify();

	m_gameState->ToggleTurnPlayer();

	while (m_gameState->GetWinState() == WinConditionReason::None)
	{
		// From here out our game is purely event driven. We simply have an input/command loop
		// which we delegate to the right places via events until the game is done.
		m_inputComponent->RequestAndProcessInput();
	}
}

//===============================================================

}
