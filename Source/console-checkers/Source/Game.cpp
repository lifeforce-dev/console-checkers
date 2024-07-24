//---------------------------------------------------------------
//
// Game.h
//

#include "Game.h"

#include "ChessLikeView.h"
#include "ConsoleInputComponent.h"
#include "GameState.h"
#include "GameTypes.h"

#include "PlayerState.h"

namespace Checkers {

//===============================================================

Game::Game()
	: m_inputComponent(std::make_unique<ConsoleInputComponent>())
	, m_gameState(std::make_unique<GameState>())
	, m_chessLikeGameView(std::make_unique<ChessLikeView>())
{
	// Creates both players.
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Red));
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Black));
}

Game::~Game() = default;

void Game::Run()
{
	m_uiPromptRequestedEvents.GetGameBoardViewStrategyChangedEvent().notify(m_chessLikeGameView.get());
	m_uiPromptRequestedEvents.GetWelcomePromptRequestedEvent().notify();

	m_gameState->ToggleTurnPlayer();

	while (m_gameState->GetWinState() == WinConditionReason::None)
	{
		// NYI
		// We essentially just request input here. Game states are moved along based on what happens
		// as a result. This loop just moves from one input to the next.
		// InputComponent will delegate the commands to the right place.
	}
}

//===============================================================

}
