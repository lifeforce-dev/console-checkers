//---------------------------------------------------------------
//
// GameSettings.h
//

#pragma once

#include "GameTypes.h"

#include <array>
#include <cstdint>

namespace Checkers {

//===============================================================

struct GameplaySettings {

	// These are the only directions that a red pawn may move.
	static constexpr std::array<glm::ivec2, 2> s_redPawnDirections = { GameBoardStatics::s_upLeft, GameBoardStatics::s_upRight };

	// All kings have the same available directions.
	static constexpr std::array<glm::ivec2, 4> s_kingDirections = { GameBoardStatics::s_upLeft, GameBoardStatics::s_upRight,
		GameBoardStatics::s_downLeft, GameBoardStatics::s_downRight };

	// These are the only directions a black pawn may move.
	static constexpr std::array<glm::ivec2, 2> s_blackPawnDirections = { GameBoardStatics::s_downLeft, GameBoardStatics::s_downRight };

	// Default Board Setup. The game board is initialized with this one.
	static constexpr std::array<Piece, 64> s_defaultGameBoard =
	{
		// row 0
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,

		// row 1
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_blackPawn, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_redPawn, GameBoardStatics::s_emptyPiece,

		// row 2
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_blackPawn, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_redPawn, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,

		// row 3
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,

		// row 4
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_blackPawn,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,

		// row 5
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_redPawn, GameBoardStatics::s_emptyPiece,

		// row 6
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_blackPawn, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_blackPawn,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_blackPawn, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece,

		// row 7
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_redKing, GameBoardStatics::s_emptyPiece,
		GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece, GameBoardStatics::s_emptyPiece
	};

	// The magnitude to apply to a direction in order to move a piece. (1 space).
	static constexpr int32_t s_moveDistance = 1;

	// The magnitude to apply to a direction in order to capture a piece. (2 spaces).
	static constexpr int32_t s_captureDistance = 2;

	// Checkers is a 2 player game.
	static constexpr int32_t s_playerCount = 2;

	// The board size is n x n.
	static constexpr int32_t s_boardSize = 8;

	// A red pawn that reaches this rank index will be promoted to king.
	static constexpr int32_t s_redKingRankIndex = 0;

	// A black pawn that reaches this rank index will be promoted to king.
	static constexpr int32_t s_blackKingRankIndex = GameplaySettings::s_boardSize - 1;

	// Official Checkers rules states that an exact game state cannot reappear 3 times, or the game ends in a draw.
	static constexpr int32_t s_gameStateRuleCount = 3;

	// Used to guide initialization of part of our game state.
	static constexpr int32_t s_theoreticalMaxCaptureCount = 12;

	// Default game board view strat.
	static constexpr GameBoardViewStrategyId s_defaultGameBoardViewStrategy = GameBoardViewStrategyId::CheckersNotation;
};

//===============================================================

}
