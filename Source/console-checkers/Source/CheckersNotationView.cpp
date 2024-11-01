//---------------------------------------------------------------
//
// CheckersNotationView.cpp
//

#include "CheckersNotationView.h"

#include "GameSettings.h"
#include "UITextStrings.h"
#include "Utility.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>

#include <sstream>

namespace Checkers {

//===============================================================

// ANSI escape code info can be found here, explains what the values 48, 38, 5, etc are:
// https://en.wikipedia.org/wiki/ANSI_escape_code

// Environment colors.
static constexpr std::string_view s_pieceBackgroundColor = "\033[48;5;233m";
static constexpr std::string_view s_hintBackgroundColor = "\033[48;5;228m";
static constexpr std::string_view s_resetColor = "\033[0m";

// Piece colors
static constexpr std::string_view s_redPawnColor = "\033[38;5;196m";
static constexpr std::string_view s_redKingColor = "\033[38;5;166m";
static constexpr std::string_view s_blackPawnColor = "\033[38;5;33m";
static constexpr std::string_view s_blackKingColor = "\033[38;5;117m";
static constexpr std::string_view s_emptySquareColor = "\033[38;5;102m";

static constexpr std::string_view s_redPawnColorHint = "\033[38;5;160m";
static constexpr std::string_view s_redKingColorHint = "\033[38;5;202m";
static constexpr std::string_view s_blackPawnColorHint = "\033[38;5;24m";
static constexpr std::string_view s_blackKingColorHint = "\033[38;5;74m";
static constexpr std::string_view s_emptySquareColorHint = "\033[38;5;239m";

// You only see this if there's an error with finding the piece color.
static constexpr std::string_view s_errorColor = "\033[38;5;206m";

// In Checkers notation, the first numbered position is 1.
static constexpr int32_t s_positionStart = 1;

static std::string_view GetColorForPiece(const Piece& piece, bool shouldShowHint)
{
	static const std::unordered_map<Piece, std::string_view, PieceHash> s_colorMap =
	{
		{ { PieceType::Empty, Identity::Neutral }, s_emptySquareColor },
		{ { PieceType::Pawn, Identity::Red }, s_redPawnColor },
		{ { PieceType::King, Identity::Red}, s_redKingColor },
		{ { PieceType::Pawn, Identity::Black }, s_blackPawnColor },
		{ { PieceType::King, Identity::Black }, s_blackKingColor },
	};

	static const std::unordered_map<Piece, std::string_view, PieceHash> s_colorHighlightMap =
	{
		{ { PieceType::Empty, Identity::Neutral }, s_emptySquareColorHint },
		{ { PieceType::Pawn, Identity::Red }, s_redPawnColorHint },
		{ { PieceType::King, Identity::Red}, s_redKingColorHint },
		{ { PieceType::Pawn, Identity::Black }, s_blackPawnColorHint },
		{ { PieceType::King, Identity::Black }, s_blackKingColorHint },
	};

	const auto& relevantColorMap = shouldShowHint ? s_colorHighlightMap : s_colorMap;
	const auto it = relevantColorMap.find(piece);
	if (it != relevantColorMap.end())
	{
		return it->second;
	}

	return s_errorColor;
}

struct CheckersNotationInputSettings {

	struct IndexMapping
	{
		// This is the value a player will input.
		int32_t notationValue = 0;

		// This is the index into the game board that the notation maps to.
		int32_t index = 0;
	};

	// An input position token has 2 values. ex. 01 or 23 or 11, etc
	static constexpr int32_t s_requiredTokenCount = 2;

	// Maps from an input index represented in checkers notation, to a game board index.
	static constexpr std::array<IndexMapping, 32> s_checkersNotationToIndexMappings =
	{
		// Row 1
		IndexMapping{ 1, 1 },IndexMapping{ 2, 3 },IndexMapping{ 3, 5 }, IndexMapping{ 4, 7 },
		// Row 2
		IndexMapping{ 5, 8 },IndexMapping{ 6, 10 },IndexMapping{ 7, 12 }, IndexMapping{ 8, 14 },
		// Row 3
		IndexMapping{ 9, 17 },IndexMapping{ 10, 19 },IndexMapping{ 11, 21 }, IndexMapping{ 12, 23 },
		// Row 4
		IndexMapping{ 13, 24 },IndexMapping{ 14, 26 },IndexMapping{ 15, 28 }, IndexMapping{ 16, 30 },
		// Row 5
		IndexMapping{ 17, 33 },IndexMapping{ 18, 35 },IndexMapping{ 19, 37 }, IndexMapping{ 20, 39 },
		// Row 6
		IndexMapping{ 21, 40 },IndexMapping{ 22, 42 },IndexMapping{ 23, 44 }, IndexMapping{ 24, 46},
		// Row 7
		IndexMapping{ 25, 49 },IndexMapping{ 26, 51 },IndexMapping{ 27, 53 }, IndexMapping{ 28, 55 },
		// Row 8
		IndexMapping{ 29, 56 },IndexMapping{ 30, 58 },IndexMapping{ 31, 60 }, IndexMapping{ 32, 62 },
	};
};

CheckersNotationView::CheckersNotationView() = default;
CheckersNotationView::~CheckersNotationView() = default;

std::string CheckersNotationView::GetGameBoardDisplayText(const std::vector<Piece>& gameBoard,
	const std::vector<int32_t>& hintIndices) const
{
	std::stringstream ss;

	int position = s_positionStart;
	for (int row = 0; row < GameplaySettings::s_boardSize; ++row)
	{
		for (int col = 0; col < GameplaySettings::s_boardSize; ++col)
		{
			// Positions are only painted onto controllable squares.
			const bool isControllableSpace = (row + col) % 2 != 0;
			if (isControllableSpace)
			{
				const int index = row * GameplaySettings::s_boardSize + col;
				const Piece& piece = gameBoard[index];

				// We will add a 0 character for alignment on values below 0.
				const bool shouldAddZero = position < 10;
				const bool isHint = std::ranges::find(hintIndices, Utility::ToGameBoardIndexFromCoord({row, col}) ) != hintIndices.end();
				// {text brush color set}{background brush set}{0 if needed}{position}{reset brush color}
				std::string coloredPiece = fmt::format("{}{}{}{}{}",
					GetColorForPiece(piece, isHint),
					isHint ? s_hintBackgroundColor : s_pieceBackgroundColor,
					shouldAddZero ? "0" : "",
					position++,
					s_resetColor);

				ss << fmt::format("[{}]", coloredPiece);
			}
			else
			{
				ss << fmt::format("{}[  ]{}", s_pieceBackgroundColor, s_resetColor);
			}
		}
		// New row.
		ss << "\n";
	}

	// Reset colors back to normal.
	ss << s_resetColor;

	return ss.str();
}

int32_t CheckersNotationView::GetGameBoardIndexFromInput(const std::string& inputPosition) const
{
	// Argument was malformed, we can't do anything with this.
	if (inputPosition.size() != CheckersNotationInputSettings::s_requiredTokenCount)
	{
		return GameBoardStatics::s_invalidBoardIndex;
	}

	int32_t inputIndex= 0;
	auto [inputAfterParse, ec] = std::from_chars(inputPosition.data(),
		inputPosition.data() + inputPosition.size(), inputIndex);
	if (ec != std::errc{})
	{
		return GameBoardStatics::s_invalidBoardIndex;
	}

	auto it = std::ranges::find_if(
		CheckersNotationInputSettings::s_checkersNotationToIndexMappings,
	[inputIndex](const CheckersNotationInputSettings::IndexMapping& mapping)
	{
		return inputIndex == mapping.notationValue;
	});

	bool found = it != CheckersNotationInputSettings::s_checkersNotationToIndexMappings.end();
	return found ? it->index : GameBoardStatics::s_invalidBoardIndex;
}

std::string_view CheckersNotationView::GetMoveCommandSyntax() const
{
	return UIText::s_moveCommandHelpExampleCheckersNotation;
}

//===============================================================

}
