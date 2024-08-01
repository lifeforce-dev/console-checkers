//---------------------------------------------------------------
//
// Utility.h
//

#pragma once

#include "GameSettings.h"
#include "GameTypes.h"

#include <cstdint>
#include <numeric>
#include <span>

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/ranges.h>

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

inline std::span<const glm::ivec2> GetDirectionsForPiece(const Piece& piece)
{
	if (piece == GameBoardStatics::s_redPawn)
	{
		return GameplaySettings::s_redPawnDirections;
	}
	if (piece == GameBoardStatics::s_blackPawn)
	{
		return GameplaySettings::s_blackPawnDirections;
	}
	if (piece == GameBoardStatics::s_blackKing || piece == GameBoardStatics::s_redKing)
	{
		return GameplaySettings::s_kingDirections;
	}
#ifdef DEBUG
	spdlog::error("Attempted to get directions for an unknown piece. piece={}", piece);
	// I want to know if we get here, it means something is wrong with our equality check.
	assert(false);
#endif

	return {};
}

inline std::string DirectionToString(const glm::ivec2& direction)
{
	if (direction == GameBoardStatics::s_up)
		return "up";
	if (direction == GameBoardStatics::s_down)
		return "down";
	if (direction == GameBoardStatics::s_left)
		return "left";
	if (direction == GameBoardStatics::s_right)
		return "right";
	if (direction == GameBoardStatics::s_upLeft)
		return "up_left";
	if (direction == GameBoardStatics::s_upRight)
		return "up_right";
	if (direction == GameBoardStatics::s_downLeft)
		return "down_left";
	if (direction == GameBoardStatics::s_downRight)
		return "downRight";
	return {};
}

inline Affinity GetPieceAffinity(const Identity sourcePieceIdentity, const Identity destPieceIdentity)
{
	if (sourcePieceIdentity == Identity::Neutral || destPieceIdentity == Identity::Neutral)
	{
		return Affinity::Neutral;
	}

	return sourcePieceIdentity == destPieceIdentity ? Affinity::Friendly : Affinity::Enemy;
}

//===============================================================

}
}
