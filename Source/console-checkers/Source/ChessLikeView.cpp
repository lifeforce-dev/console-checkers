//---------------------------------------------------------------
//
// ChessLikeView.cpp
//

#include "ChessLikeView.h"

#include "GameSettings.h"
#include "GameTypes.h"
#include "UITextStrings.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/ranges.h>

#include <cassert>
#include <ranges>
#include <sstream>

namespace Checkers {

//===============================================================

// Ranks are labeled from 8 to 1 on the side of the board.
static constexpr unsigned char s_rankStartGlyph = '8';

// Files are labeled from A to H on the side of the board.
static constexpr unsigned char s_fileStartGlyph = 'A';

// A custom hash function is used to hash a piece and map to a visual representation.
static const std::unordered_map<Piece, char, PieceHash> s_glyphMap = {
	{{PieceType::Empty, Identity::Neutral}, ' '},
	{{PieceType::Pawn, Identity::Red}, 'r'},
	{{PieceType::King, Identity::Red}, 'R'},
	{{PieceType::Pawn, Identity::Black}, 'b'},
	{{PieceType::King, Identity::Black}, 'B'},
};

// Returns the string representation of an entire piece, including the [].
static std::string GetGlyphForPiece(const Piece& piece) {

	const auto it = s_glyphMap.find(piece);
	if (it != s_glyphMap.end()) {
		return fmt::format(UIText::s_glyphBoardSlot, it->second);
	}

	return fmt::format(UIText::s_glyphBoardSlot, "ERR");
}

// Returns the character count of the entire piece, including the [].
static size_t GetPieceGlyphSize()
{
	const size_t glyphSize = GetGlyphForPiece({ PieceType::Empty, Identity::Neutral }).size();

#ifdef DEBUG
	// Our UI formatting depends on each piece being the same display size. Assert only in debug.
	for (char glyph : s_glyphMap | std::views::values) {
		std::string formatted = fmt::format(UIText::s_glyphBoardSlot, glyph);
		assert(formatted.length() == glyphSize && "Formatted glyph must be 3 characters long");
	}
#endif

	return glyphSize;
}


ChessLikeView::ChessLikeView() = default;
ChessLikeView::~ChessLikeView() = default;

std::string ChessLikeView::GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const
{
	std::stringstream ss;

	unsigned char currentColumnHeader = s_fileStartGlyph;

	// Pad the columnHeader for alignment.
	ss << "  ";
	for (int32_t col = 0; col < GameplaySettings::s_boardSize; ++col)
	{
		// We want to align the header values according to the center of each piece.
		ss << currentColumnHeader++ << std::string(GetPieceGlyphSize() - 1, ' ');
	}

	ss << "\n";

	// Print the actual game board
	unsigned char currentRowHeader = s_rankStartGlyph;
	for (int row = 0; row < GameplaySettings::s_boardSize; ++row)
	{
		ss << currentRowHeader--;
		for (int col = 0; col < GameplaySettings::s_boardSize; ++col)
		{
			const int index = row * GameplaySettings::s_boardSize + col;
			ss << GetGlyphForPiece(gameBoard[index]);
		}

		ss << "\n";
	}

	return fmt::format("{}\n", ss.str());
}

int32_t ChessLikeView::GameBoardPositionToIndex(const std::string& position) const
{
	// NYI
	return 0;
}

std::string_view ChessLikeView::GetMoveCommandSyntax() const
{
	return UIText::s_moveCommandHelpExampleChessLikeView;
}

//===============================================================

}
