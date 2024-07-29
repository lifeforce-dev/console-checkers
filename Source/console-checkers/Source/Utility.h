//---------------------------------------------------------------
//
// Utility.h
//

#pragma once

#include "GameSettings.h"
#include "GameTypes.h"

#include <cstdint>
#include <numeric>

namespace Checkers {
namespace Utility {

//===============================================================

// Returns whether this index fits on the board even.
inline bool IsValidGameBoardIndex(int32_t index)
{
	constexpr int32_t s_gameSquareCount = GameplaySettings::s_boardSize * GameplaySettings::s_boardSize;
	return index >= 0 && index < s_gameSquareCount;
}


// Returns a flat array index to our game board.
inline int32_t ToGameBoardIndexFromCoord(const glm::ivec2& index)
{
	return index.x * GameplaySettings::s_boardSize + index.y;
}

// Given a flat array index to our game board, returns its coordinate.
inline glm::ivec2 ToGameBoardCoordFromIndex(int32_t index)
{
	return { index / GameplaySettings::s_boardSize, index % GameplaySettings::s_boardSize };
}

inline bool IsValidGameBoardCoord(const glm::ivec2& coord)
{
	return coord.x >= 0 && coord.x < GameplaySettings::s_boardSize &&
		coord.y >= 0 && coord.y < GameplaySettings::s_boardSize;
}

inline glm::ivec2 GetMoveDestCoord(const glm::ivec2& source, const glm::ivec2& direction)
{
	return source + GameplaySettings::s_moveDistance * direction;
}

inline glm::ivec2 GetCaptureDestCoord(const glm::ivec2& source, const glm::ivec2& direction)
{
	return source + GameplaySettings::s_captureDistance * direction;
}

// Returns the diagonal distance between a source and a dest. A capture should be 2, a move should be 1.
inline int32_t GetDistanceFromSourceIndex(int32_t sourceIndex, int32_t destIndex)
{
	const glm::ivec2 sourceCoord = ToGameBoardCoordFromIndex(sourceIndex);
	const glm::ivec2 destCoord = ToGameBoardCoordFromIndex(destIndex);
	const int32_t rowDiff = std::abs(sourceCoord.x - destCoord.x);
	const int32_t colDiff = std::abs(sourceCoord.y - destCoord.y);

	// Diagonal distance is the maximum of the row and column differences - Math.
	return std::max(rowDiff, colDiff);
}

inline glm::ivec2 NormalizeDirection(const glm::ivec2& direction)
{
	return { direction.x / abs(direction.x), direction.y / abs(direction.y) };
}

inline glm::ivec2 GetDirectionBetweenTwoIndices(int32_t sourceIndex, int32_t destIndex)
{
	const glm::ivec2 sourceCoord = Utility::ToGameBoardCoordFromIndex(sourceIndex);
	const glm::ivec2 destCoord = Utility::ToGameBoardCoordFromIndex(destIndex);
	const glm::ivec2 direction = destCoord - sourceCoord;

	return NormalizeDirection(direction);
}

//===============================================================

}
}
