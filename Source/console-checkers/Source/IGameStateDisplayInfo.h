//---------------------------------------------------------------
//
// IGameStateDisplayInfo.h
//

#pragma once

#include "GameTypes.h"
#include <vector>

namespace Checkers {

//===============================================================

class IGameStateDisplayInfo
{
public:
	IGameStateDisplayInfo(const IGameStateDisplayInfo& other) = delete;
	IGameStateDisplayInfo& operator=(const IGameStateDisplayInfo& other) = delete;

	IGameStateDisplayInfo(IGameStateDisplayInfo&& other) noexcept = default;
	IGameStateDisplayInfo& operator=(IGameStateDisplayInfo&& other) noexcept = default;
	IGameStateDisplayInfo() = default;
	virtual ~IGameStateDisplayInfo() = default;


	// Returns the game data that will be rendered for the player.
	const virtual std::vector<Piece>& GetGameBoardData() const = 0;

	// Returns the list of pieces captured by black player.
	const virtual std::vector<Piece>& GetBlackPlayerCapturedPieces() const = 0;

	// Returns the list of pieces captured by red player.
	const virtual std::vector<Piece>& GetRedPlayerCapturedPieces() const = 0;

	// Returns the identity of the player whose turn it currently is.
	const virtual Identity GetTurnPlayerId() const = 0;
};

//===============================================================

}
