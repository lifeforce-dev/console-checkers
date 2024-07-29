//---------------------------------------------------------------
//
// GameTypes.h
//

#pragma once

#include <cstdint>
#include <functional>
#include <ostream>

#include "glm/vec2.hpp"

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

inline std::ostream& operator<<(std::ostream&os, PieceType pieceType)
{
	switch (pieceType)
	{
	case PieceType::Empty:
		os << "Empty";
		break;
	case PieceType::Pawn:
		os << "Pawn";
		break;
	case PieceType::King:
		os << "King";
		break;
	default:
		os << "Unknown";
		break;
	}
	return os;
}

// Determines how its owner can interact with the game as well as is displayed.
enum class Identity : int32_t
{
	// Empty spaces will be neutral to any other piece.
	Neutral,

	// Black player will be displayed as black and moves second.
	Black,

	// Red player will be displayed as red and move first.
	Red,
};

// TODO: these strings should use UIText::
inline std::ostream& operator<<(std::ostream& os, Identity id)
{
	switch (id)
	{
	case Identity::Neutral:
		os << "Neutral";
		break;
	case Identity::Black:
		os << "Black Player";
		break;
	case Identity::Red:
		os << "Red Player";
		break;
	default:
		os << "Unknown";
		break;
	}
	return os;
}

struct BoardDirectionStatics
{
	// 2d array index directions go by row col.
	static constexpr glm::ivec2 s_up = glm::ivec2(-1, 0);
	static constexpr glm::ivec2 s_down = glm::ivec2(1, 0);
	static constexpr glm::ivec2 s_left = glm::ivec2(0, -1);
	static constexpr glm::ivec2 s_right = glm::ivec2(0, 1);

	// Define diagonal directions using primary directions
	static constexpr glm::ivec2 s_upRight = s_up + s_right;
	static constexpr glm::ivec2 s_upLeft = s_up + s_left;
	static constexpr glm::ivec2 s_downRight = s_down + s_right;
	static constexpr glm::ivec2 s_downLeft = s_down + s_left;
};

// Inspired by boost, simple hash combine for hashing custom structs.
template <class T>
inline void hash_combine(std::size_t& s, const T& v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

struct PieceMoveDescription
{
	// Index into the game board where the source piece is.
	int32_t sourceIndex = 0;

	// Index into the game board where the piece will be moved to.
	int32_t destIndex = 0;

	bool operator==(const PieceMoveDescription& other) const
	{
		return sourceIndex == other.sourceIndex && destIndex == other.destIndex;
	}
};
inline std::ostream& operator<<(std::ostream& os, const PieceMoveDescription& move)
{
	os << "[" << move.sourceIndex << ", " << move.destIndex << "]";
	return os;
}

struct PieceMoveDescriptionHash
{
	std::size_t operator()(const PieceMoveDescription& move) const
	{
		const std::size_t sourceHash = std::hash<int32_t>()(move.sourceIndex);
		const std::size_t destHash = std::hash<int32_t>()(move.destIndex);

		std::size_t hash = 0;
		hash_combine(hash, move.sourceIndex);
		hash_combine(hash, move.destIndex);
		// Combine the hashes with bitwise operations and use another prime number for better distribution.
		return hash;
	}
};

struct PieceMoveHint
{
	// The size of this chain will serve as the score of this move hint.
	// The list of moves resulting in maximal captures that can be chained to the base move.
	std::vector<PieceMoveDescription> captureChain;
};

enum class MoveEvaluation : int32_t
{
	// The move was not a valid one in any capacity.
	Invalid,

	// The player is able to move their piece (1 space).
	MoveAvailable,

	// The move is a capture (2 spaces).
	CaptureAvailable
};

struct Piece
{
	// Determines how the piece can move.
	PieceType pieceType;

	// Determines when this piece can move.
	Identity identity;

	// The combined Piece Type and Identity make up the logical identity of a piece.
	// Therefore, those are the only values that we include in the hash and equality operator.
	bool operator==(const Piece& other) const
	{
		return pieceType == other.pieceType && identity == other.identity;
	}
};
inline std::ostream& operator<<(std::ostream& os, const Piece& piece)
{
	os << "PieceType: " << piece.pieceType << ", Identity: " << piece.identity;
	return os;
}

struct PieceHash
{
	std::size_t operator()(const Piece& piece) const
	{
		std::size_t hash = 0;
		hash_combine(hash, piece.pieceType);
		hash_combine(hash, piece.identity);
		// ^ is an XOR and Combines the two together ensuring that changes in one significantly changes the output of the final hash
		// the multiplication by 3 uses a small prime number to modify one of the hashes before combining, which helps distribution.
		return hash;
	}
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

enum class WinConditionReason : int32_t
{
	// No win condition determined.
	None,

	// A player has captured all enemy pieces.
	AllEnemyPiecesCapturedWin,

	// The player finds themself with pieces but nowhere to move them.
	NoAvailableMovesLoss,

	// The exact same game state has been achieved 3 times.
	GameStateViolationDraw
};

// A view strategy is responsible for implementing and translating between
// what the player sees and what the game understands. This enum identifies a
// particular view strategy.
enum class GameBoardViewStrategyId
{
	// Start of the options.
	Invalid = 0,

	// Displays board with chess notation and glyphs, and understand how to interpret input in that format.
	ChessLikeView,

	// Displays board with checkers notation, and understand how to interpret input in that format.
	CheckersNotation,

	// Number of options.
	End = 3
};

// TODO: these strings should use UIText::
inline std::ostream& operator<<(std::ostream& os, GameBoardViewStrategyId id)
{
	switch (id)
	{
	case GameBoardViewStrategyId::Invalid:
		os << "Invalid";
		break;
	case GameBoardViewStrategyId::ChessLikeView:
		os << "ChessLikeView";
		break;
	case GameBoardViewStrategyId::CheckersNotation:
		os << "CheckersNotation";
		break;
	case GameBoardViewStrategyId::End:
		os << "(End) Invalid";
		break;
	default:
		os << "Invalid";
		break;
	}
	return os;
}

struct GameBoardStatics
	{
	// Often used for failure to get a valid index.
	static constexpr int32_t s_invalidBoardIndex = -1;

	// Static helper pieces we can check against when performing game logic.
	static constexpr Piece s_redPawn { PieceType::Pawn, Identity::Red };
	static constexpr Piece s_redKing{ PieceType::King, Identity::Red };
	static constexpr Piece s_blackPawn{ PieceType::Pawn, Identity::Black };
	static constexpr Piece s_blackKing{ PieceType::King, Identity::Black };
	static constexpr Piece s_emptyPiece{ PieceType::Empty, Identity::Neutral };
};

struct GameStateRecord
{
	Identity playerIdentity;
	std::size_t gameBoardHash;
	bool operator==(const GameStateRecord& other) const
	{
		return playerIdentity == other.playerIdentity && gameBoardHash== other.gameBoardHash;
	}
};

struct GameStateRecordHash
{
	std::size_t operator()(const GameStateRecord& record) const
	{
		std::size_t hash = 0;
		hash_combine(hash, record.playerIdentity);
		hash_combine(hash, record.gameBoardHash);

		return hash;
	}
};
//===============================================================

}

// Need to tell raw hashes of Piece to use PieceHash.
namespace std {

	template <>
	struct hash<Checkers::Piece>
	{
		std::size_t operator()(const Checkers::Piece& piece) const
		{
			return Checkers::PieceHash{}(piece);
		}
	};
}
