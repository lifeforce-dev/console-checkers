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
	const virtual std::vector<Piece>& GetGameBoardData() = 0;
};

//===============================================================

}
