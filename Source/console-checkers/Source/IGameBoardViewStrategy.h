//---------------------------------------------------------------
//
// IGameBoardViewStrategy.h
//

#pragma once

#include "GameTypes.h"

#include <string>

namespace Checkers {

//===============================================================

// The view changes how the player sees the data representation, and therefore
// how they interact.

// In console programs, the display is heavily coupled with the input. Because of this,
// we have an interface that is familiar with both and can tie them together so that the
// rest of the game can understand how to interact with a particular view.
class IGameBoardViewStrategy {
public:
	IGameBoardViewStrategy(const IGameBoardViewStrategy& other) = delete;
	IGameBoardViewStrategy& operator=(const IGameBoardViewStrategy& other) = delete;

	IGameBoardViewStrategy() = default;
	IGameBoardViewStrategy(IGameBoardViewStrategy&& other) noexcept = default;
	IGameBoardViewStrategy& operator=(IGameBoardViewStrategy&& other) noexcept = default;
	virtual ~IGameBoardViewStrategy() = default;

	// Gets the textual representation of the game board data.
	virtual std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const = 0;

	// Takes a formatted player-facing board position string and returns a game board index.
	virtual int32_t GameBoardPositionToIndex(const std::string& position) const = 0;

	// Gets the move command syntax for this particular view.
	virtual std::string_view GetMoveCommandSyntax() const = 0;
};

//===============================================================

}
