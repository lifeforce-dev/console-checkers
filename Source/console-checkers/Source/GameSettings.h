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

	// The default board configuration. If you need to do some testing, you can make changes here and they will be reflected on startup.
	static constexpr std::array<Piece, 64> s_defaultGameBoard =
	{
		// Top three rows for 'Black' pieces (farthest from player perspective).
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black},

		Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral},

		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Black}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Black},

		// Middle two rows are empty.
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},

		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Empty, Identity::Neutral},

		// Bottom three rows for 'Red' pieces (closest to player perspective).
		Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral},

		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red},

		Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red},
		Piece{PieceType::Empty, Identity::Neutral}, Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral},
		Piece{PieceType::Pawn, Identity::Red}, Piece{PieceType::Empty, Identity::Neutral}
	};

	// Checkers is a 2 player game.
	static constexpr int32_t s_playerCount = 2;

	// The board size is n x n.
	static constexpr int32_t s_boardSize = 8;
};

struct GameMoveCommandSettings {
	// The number of positions described in the command. ex. a1 b3 is 2
	static constexpr int32_t s_posCount = 2;

	// The number of tokens in a position. ex. a1 has 2 tokens.
	static constexpr int32_t s_posTokenCount = 2;

	// The index of the column token within the current position. ex. b6 b is the column.
	static constexpr int32_t s_colTokenIndex = 0;

	// The index of the row token within the current position. ex. b7, 1 is 7 is the row.
	static constexpr int32_t s_rowTokenIndex = 1;

	struct TokenRange
	{
		unsigned char min;
		unsigned char max;
	};

	// Input tokens for rows on moves cannot be outside of this range.
	static constexpr TokenRange s_rowTokenRange{ '1', '8' };

	// Input tokens for cols on moves cannot be outside of this range.
	static constexpr TokenRange s_colTokenRange{ 'a', 'h' };
};

//===============================================================

}
