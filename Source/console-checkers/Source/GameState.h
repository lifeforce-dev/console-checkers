//---------------------------------------------------------------
//
// GameState.h
//

#pragma once

#include "IGameStateDisplayInfo.h"

#include "GameTypes.h"

#include <vector>

namespace Checkers {

//===============================================================

class GameState : public IGameStateDisplayInfo
{
public:
	GameState(const GameState& other) = delete;
	GameState& operator=(const GameState& other) = delete;

	GameState(GameState&& other) noexcept = default;
	GameState& operator=(GameState&& other) noexcept = default;

	GameState();
	virtual ~GameState() override;

	// Raw view of our game board.
	const std::vector<Piece>& GetGameBoardData() override { return m_gameBoard; }

	// Anything other than None indicates a win condition has been met, and describes how.
	WinConditionReason GetWinState() const { return m_winState; }

	// Changes the turn player back and forth, or sets the initial player when called for the first time.
	void ToggleTurnPlayer();

private:

	// Indicates whose turn it currently is.
	Identity m_turnPlayer = Identity::Neutral;

	// The checkers board data.
	std::vector<Piece> m_gameBoard;

	// Current win state of the game
	WinConditionReason m_winState = WinConditionReason::None;
};

//===============================================================

}
