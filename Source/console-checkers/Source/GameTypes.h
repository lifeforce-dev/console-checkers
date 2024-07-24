//---------------------------------------------------------------
//
// GameTypes.h
//

#pragma once

#include <cstdint>
#include <functional>
#include <ostream>

namespace Checkers {

//===============================================================

enum class PieceType : int32_t
{
	// Indicates no piece.
	Empty,

	// Indicates the piece is a pawn.
	Pawn,

	// Indicates the piece is a king.
	King
};

// Determines how its owner can interact with the game as well as is displayed.
enum class Identity : int32_t
{
	// Empty spaces will be neutral.
	Neutral,

	// Black player will be displayed as black and moves second.
	Black,

	// Red player will be displayed as red and move first.
	Red,
};

inline std::ostream& operator<<(std::ostream& os, Identity id)
{
	switch (id)
	{
	case Identity::Neutral:
		os << "Neutral";
		break;
	case Identity::Black:
		os << "Black";
		break;
	case Identity::Red:
		os << "Red";
		break;
	default:
		os << "Unknown";
		break;
	}
	return os;
}

struct Piece
{
	// Determines how the piece can move.
	PieceType pieceType;

	// Determines when this piece can move.
	Identity identity;

	bool operator==(const Piece& other) const
	{
		return pieceType == other.pieceType && identity == other.identity;
	}
};

struct PieceHash {
	std::size_t operator()(const Piece& k) const
	{
		// ^ is an XOR and Combines the two together ensuring that changes in one significantly changes the output of the final hash
		// the multiplication by 3 uses a small prime number to modify one of the hashes before combining, which helps distribution.
		return std::hash<int>()(static_cast<int>(k.pieceType)) ^ (std::hash<int>()(static_cast<int>(k.identity)) * 3);
	}
};

enum class WinConditionReason : int32_t
{
	// No win condition determined.
	None,

	// A player has captured all enemy pieces.
	AllEnemyPiecesCapturedWin,

	// Player has achieved a board state in which enemy has pieces but cannot move them.
	EnemyHasNoAvailableMovesWin,

	// Neither player has any available moves left.
	NoAvailableMovesDraw,

	// The exact same game state has been achieved 3 times.
	GameStateViolationDraw
};


// Used in capture logic to determine whether a capture is available.
enum class Affinity : int32_t
{
	// This is really only possible by comparing two neutral pieces.
	Neutral,

	// Two pieces owned by players of differing colors are enemies.
	Enemy,

	// Two pieces owned by players of same colors are friendlies.
	Friendly
};

//===============================================================

}
