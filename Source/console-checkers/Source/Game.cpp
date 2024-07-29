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

#include <spdlog/spdlog.h>

// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>

namespace Checkers {

//===============================================================

GameBoardViewStrategyRegistry::GameBoardViewStrategyRegistry()
{
	m_registeredGameBoardViews.push_back(std::make_unique<CheckersNotationView>());
	m_registeredGameBoardViews.push_back(std::make_unique<ChessLikeView>());
}

const std::vector<std::unique_ptr<IGameBoardViewStrategy>>& GameBoardViewStrategyRegistry::GetRegisteredViews() const
{
	return m_registeredGameBoardViews;
}

bool GameBoardViewStrategyRegistry::IsRegisteredView(GameBoardViewStrategyId id) const
{
	auto it = std::find_if(std::cbegin(m_registeredGameBoardViews),
		std::cend(m_registeredGameBoardViews),
		[id](const std::unique_ptr<IGameBoardViewStrategy>& view)
	{
		return view->GetId() == id;
	});

	return it != std::cend(m_registeredGameBoardViews);
}

IGameBoardViewStrategy* GameBoardViewStrategyRegistry::GetGameBoardViewStrategyForId(GameBoardViewStrategyId id) const
{
	auto it = std::find_if(std::cbegin(m_registeredGameBoardViews),
		std::cend(m_registeredGameBoardViews),
		[id](const std::unique_ptr<IGameBoardViewStrategy>& view)
	{
		return view->GetId() == id;
	});

	if (it != std::cend(m_registeredGameBoardViews))
	{
		return it->get();
	}

#ifdef DEBUG
	// Hi! If you're here, you forgot to add the view to the registry :).
	assert(false);
#endif

	spdlog::error("Attempted to use unregistered game view strategy. id={}", id);
	return nullptr;
}

IGameBoardViewStrategy* GameBoardViewStrategyRegistry::GetGameBoardViewStrategyForPlayerOption(int32_t option) const
{
	// Options are 1 based because they are player-facing. We subtract one for indexing.
	option--;

	if (option < 0 || option > static_cast<int32_t>(m_registeredGameBoardViews.size()))
	{
		// This can happen if the player selected an option that we deemed to be a valid number, but doesn't
		// actually map to anything we have registered.
		return nullptr;
	}
	return m_registeredGameBoardViews[option].get();
}

//---------------------------------------------------------------

Game::Game()
	: m_inputComponent(std::make_unique<ConsoleInputComponent>(this))
	, m_gameState(std::make_unique<GameState>(this))
	, m_gameBoardViewStrategyRegistry(std::make_unique<GameBoardViewStrategyRegistry>())
{
	m_selectedGameBoardViewStrategy = m_gameBoardViewStrategyRegistry->GetGameBoardViewStrategyForId(
		GameplaySettings::s_defaultGameBoardViewStrategy);

	// TODO: Its not obvious that we CANNOT fire off UI events in here. Display
	// is not alive yet. But we don't know that. Our owner should probably tell us.
	// Or Display could via UI events but that seems a bit out of place.
}

Game::~Game() = default;

void Game::SetSelectedGameBoardViewStrategy(GameBoardViewStrategyId id)
{
	if (m_selectedGameBoardViewStrategy->GetId() == id)
	{
		return;
	}

	m_selectedGameBoardViewStrategy = m_gameBoardViewStrategyRegistry->GetGameBoardViewStrategyForId(id);

	// Our UI needs to be notified of the change.
	m_uiPromptRequestedEvents.GetGameBoardViewStrategyChangedEvent().notify(m_selectedGameBoardViewStrategy);
}

void Game::MovePiece(const PieceMoveDescription& moveDescription) const
{
	m_gameState->MovePiece(moveDescription);
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
