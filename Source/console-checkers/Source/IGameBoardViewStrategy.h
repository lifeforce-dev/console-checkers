//---------------------------------------------------------------
//
// IGameBoardViewStrategy.h
//

#pragma once

#include "GameTypes.h"

#include <string>

namespace Checkers {

//===============================================================

class IGameBoardViewStrategy {
public:
	IGameBoardViewStrategy(const IGameBoardViewStrategy& other) = delete;
	IGameBoardViewStrategy& operator=(const IGameBoardViewStrategy& other) = delete;

	IGameBoardViewStrategy() = default;
	virtual ~IGameBoardViewStrategy() = default;
	IGameBoardViewStrategy(IGameBoardViewStrategy&& other) noexcept = default;
	IGameBoardViewStrategy& operator=(IGameBoardViewStrategy&& other) noexcept = default;

	// Gets the textual representation of the game board data.
	[[nodiscard]]
	virtual std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const = 0;

	// Takes a formatted player-facing board position string and returns a game board index.
[[nodiscard]]
	virtual int32_t GameBoardPositionToIndex(const std::string& position) const = 0;
};

//===============================================================

}
