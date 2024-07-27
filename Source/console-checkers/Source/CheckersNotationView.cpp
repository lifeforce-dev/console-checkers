//---------------------------------------------------------------
//
// CheckersNotationView.cpp
//

#include "CheckersNotationView.h"

#include "GameSettings.h"
#include "UITextStrings.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/format.h"

#include <sstream>

namespace Checkers {

//===============================================================

// ANSI escape code info can be found here, explains what the values 48, 38, 5, etc are:
// https://en.wikipedia.org/wiki/ANSI_escape_code

// Environment colors.
static constexpr std::string_view s_pieceBackgroundColor = "\033[48;5;233m";
static constexpr std::string_view s_resetColor = "\033[0m";

// Piece colors
static constexpr std::string_view s_redPawnColor = "\033[38;5;196m";
static constexpr std::string_view s_redKingColor = "\033[38;5;166m";
static constexpr std::string_view s_blackPawnColor = "\033[38;5;33m";
static constexpr std::string_view s_blackKingColor = "\033[38;5;117m";
static constexpr std::string_view s_emptySquareColor = "\033[38;5;102m";

// You only see this if there's an error with finding the piece color.
static constexpr std::string_view s_errorColor = "\033[38;5;206m";

// In Checkers notation, the first numbered position is 1.
static constexpr int32_t s_positionStart = 1;

static std::string_view GetColorForPiece(const Piece& piece)
{
	static const std::unordered_map<Piece, std::string_view, PieceHash> s_colorMap =
	{
		{ { PieceType::Empty, Identity::Neutral }, s_emptySquareColor },
		{ { PieceType::Pawn, Identity::Red }, s_redPawnColor },
		{ { PieceType::King, Identity::Red}, s_redKingColor },
		{ { PieceType::Pawn, Identity::Black }, s_blackPawnColor },
		{ { PieceType::King, Identity::Black }, s_blackKingColor },
	};

	const auto it = s_colorMap.find(piece);
	if (it != s_colorMap.end())
	{
		return it->second;
	}

	return s_errorColor;
}


CheckersNotationView::CheckersNotationView() = default;
CheckersNotationView::~CheckersNotationView() = default;

std::string CheckersNotationView::GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const
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

				// {text brush color set}{background brush set}{0 if needed}{position}{reset brush color}
				std::string coloredPiece = fmt::format("{}{}{}{}{}",
					GetColorForPiece(piece),
					s_pieceBackgroundColor,
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

int32_t CheckersNotationView::GameBoardPositionToIndex(const std::string& position) const
{
	// NYI
	return 0;
}

std::string_view CheckersNotationView::GetMoveCommandSyntax() const
{
	return UIText::s_moveCommandHelpExampleCheckersNotation;
}

//===============================================================

}
