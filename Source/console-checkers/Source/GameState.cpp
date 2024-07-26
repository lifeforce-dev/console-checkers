//---------------------------------------------------------------
//
// GameState.cpp
//

#include "GameSettings.h"
#include "GameState.h"

#include <spdlog/spdlog.h>

// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>

namespace Checkers {

//===============================================================

GameState::GameState()
	: m_gameBoard(GameplaySettings::s_defaultGameBoard.size())
{
	// Init our board with what we have in settings. This helps with debugging as well.
	std::ranges::copy(GameplaySettings::s_defaultGameBoard,
		std::begin(m_gameBoard));
}

GameState::~GameState() = default;

void GameState::ToggleTurnPlayer()
{
	Identity oldTurnPlayer = m_turnPlayer;

	if (m_turnPlayer == Identity::Neutral)
	{
		// First turn has started, red always goes first.
		m_turnPlayer = Identity::Red;

		// TODO: GetPlayerTurnChangedEvent().notify()
	}
	else
	{
		m_turnPlayer = (m_turnPlayer == Identity::Red) ? Identity::Black : Identity::Red;
	}

	spdlog::info("Player turn change. old {} player new {} player", oldTurnPlayer, m_turnPlayer);
}

//===============================================================

}
