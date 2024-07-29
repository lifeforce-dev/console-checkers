//---------------------------------------------------------------
//
// ChessLikeView.cpp
//

#include "ChessLikeView.h"

#include "GameSettings.h"
#include "GameTypes.h"
#include "UITextStrings.h"
#include "Utility.h"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <ranges>
#include <sstream>

namespace Checkers {

//===============================================================

struct GameBoardInputPosSettings
{
	// A board position has two tokens. eg. a5
	static constexpr int32_t requiredTokenCount = 2;

	// The index of the column token within the current position. ex. b6 b is the column.
	static constexpr int32_t s_colTokenIndex = 0;

	// The index of the row token within the current position. ex. b7, 1 is 7 is the row.
	static constexpr int32_t s_rowTokenIndex = 1;

	struct Range {
		char min;
		char max;
	};
	static constexpr Range s_colTokenRange{ 'a', 'h' };
	static constexpr Range s_rowTokenRange{ '1', '8' };
};

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

	// Pad the columnHeader for alignment.
	ss << "  ";
	for (int32_t col = 0; col < GameplaySettings::s_boardSize; ++col)
	{
		// We want to align the header values according to the center of each piece.
		ss << currentColumnHeader++ << std::string(GetPieceGlyphSize() - 1, ' ');
	}

	ss << "\n";
	return fmt::format("{}\n", ss.str());
}



int32_t ChessLikeView::GetGameBoardIndexFromInput(const std::string& inputPos) const
{
	if (inputPos.size() != GameBoardInputPosSettings::requiredTokenCount)
	{
		spdlog::error("ChessLike inputPos position malformed. inputPos={}", inputPos);
		return GameBoardStatics::s_invalidBoardIndex;
	}

	// C++ 17 structured binding.
	auto [inputCol, inputRow] = std::pair{ inputPos[GameBoardInputPosSettings::s_colTokenIndex],
		inputPos[GameBoardInputPosSettings::s_rowTokenIndex] };

// rowIndex looks weird for a couple reasons. In addition to converting to int we need to:
	// 1) Invert the numbering since 0,0 is at the top for us, and A,1 is at the bottom for display.
	// 2) Add one to our min range when calculating to account for 0based vs 1based numbering.
	// 3) Translate to a flat array.

	//                                inverts numbering order  (converts row char to index int   (account for 0 based indexing))
	const int32_t rowIndex = GameplaySettings::s_boardSize - (inputRow - (GameBoardInputPosSettings::s_rowTokenRange.min - 1));

	// Math for getting the col index from the column letter.
	const int32_t colIndex = inputCol - GameBoardInputPosSettings::s_colTokenRange.min;

	// Translate to flat array.
	const int32_t index = rowIndex * GameplaySettings::s_boardSize + colIndex;

	return Utility::IsValidGameBoardIndex(index) ? index : GameBoardStatics::s_invalidBoardIndex;
}

std::string_view ChessLikeView::GetMoveCommandSyntax() const
{
	return UIText::s_moveCommandHelpExampleChessLikeView;
}

//===============================================================

}
