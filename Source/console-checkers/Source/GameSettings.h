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
	static constexpr std::array<glm::ivec2, 2> s_redPawnDirections = { BoardDirectionStatics::s_upLeft, BoardDirectionStatics::s_upRight };

	// All kings have the same available directions.
	static constexpr std::array<glm::ivec2, 4> s_kingDirections = { BoardDirectionStatics::s_upLeft, BoardDirectionStatics::s_upRight,
		BoardDirectionStatics::s_downLeft, BoardDirectionStatics::s_downRight };

	// These are the only directions a black pawn may move.
	static constexpr std::array<glm::ivec2, 2> s_blackPawnDirections = { BoardDirectionStatics::s_downLeft, BoardDirectionStatics::s_downRight };

	// The default board configuration. If you need to do some testing, you can make changes here and they will be reflected on startup.
		// The default board configuration. If you need to do some testing, you can make changes here and they will be reflected on startup.
	static constexpr std::array<Piece, 64> s_defaultGameBoard =
	{
		// Top three rows for 'Black' pieces (farthest from player perspective).
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Black }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Black }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Black },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{PieceType::Pawn, Identity::Black},

		Piece{ PieceType::Pawn, Identity::Black }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Black },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Black }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Black }, Piece{PieceType::Empty, Identity::Neutral },

		Piece{ PieceType::Empty, Identity::Neutral }, Piece{PieceType::Pawn, Identity::Black }, Piece{PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Black }, Piece{PieceType::Empty, Identity::Neutral }, Piece{PieceType::Pawn, Identity::Black },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{PieceType::Pawn, Identity::Black },

		// Middle two rows are empty.
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},

		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{PieceType::Empty, Identity::Neutral },

		// Bottom three rows for 'Red' pieces (closest to player perspective).
		Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral },

		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red },

		Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red },
		Piece{ PieceType::Empty, Identity::Neutral }, Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral },
		Piece{ PieceType::Pawn, Identity::Red }, Piece{ PieceType::Empty, Identity::Neutral }
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

	// Default game board view strat.
	static constexpr GameBoardViewStrategyId s_defaultGameBoardViewStrategy = GameBoardViewStrategyId::CheckersNotation;
};

//===============================================================

}
