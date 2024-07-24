//---------------------------------------------------------------
//
// ConsoleGameDisplay.cpp
//

#include "ConsoleGameDisplay.h"

#include "IGameStateDisplayInfo.h"
#include "GameSettings.h"
#include "GameTypes.h"
#include "UIPromptRequestedEvents.h"
#include "UITextStrings.h"

#include <iostream>
#include <ranges>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>


namespace Checkers {

//===============================================================

static constexpr unsigned char s_rankStartGlyph = '8';
static constexpr unsigned char s_fileStartGlyph = 'A';

static const std::unordered_map<Piece, char, PieceHash> s_glyphMap = {
	{{PieceType::Empty, Identity::Neutral}, ' '},
	{{PieceType::Pawn, Identity::Red}, 'r'},
	{{PieceType::King, Identity::Red}, 'R'},
	{{PieceType::Pawn, Identity::Black}, 'b'},
	{{PieceType::King, Identity::Black}, 'B'},
};

static std::string GetGlyphForPiece(const Piece& piece) {

	auto it = s_glyphMap.find(piece);
	if (it != s_glyphMap.end()) {
		return fmt::format(UIText::s_glyphBoardSlot, it->second);
	}

	return fmt::format(UIText::s_glyphBoardSlot, "ERR");
}

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

//---------------------------------------------------------------

ConsoleGameDisplay::ConsoleGameDisplay()
{
}

void ConsoleGameDisplay::Initialize(IGameStateDisplayInfo* displayData, UIPromptRequestedEvents& events)
{
	m_gameStateInfo = displayData;

	events.GetWelcomePromptRequestedEvent().subscribe([this]()
	{
		std::string uiScreenText = fmt::format("{}\n\n\n", UIText::s_welcomeMessage);
		uiScreenText.append(GetGameBoardDisplay());

		std::cout << uiScreenText;
	});

	m_isInitialized = true;
}

std::string ConsoleGameDisplay::GetGameBoardDisplay() const
{
#ifdef DEBUG
	// We should not be calling any functions on ConsoleGameDisplay unless we're initialized.
	assert(m_isInitialized);
#endif

	const std::vector<Piece>& gameBoard = m_gameStateInfo->GetGameBoardData();

	std::stringstream ss;
	unsigned char currentColumnHeader = s_fileStartGlyph;

	// Pad the columnHeader for alignment.
	ss << "  ";
	for (int32_t col = 0; col < GameplaySettings::s_boardSize; ++col)
	{
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
			int index = row * GameplaySettings::s_boardSize + col;
			ss << GetGlyphForPiece(gameBoard[index]);
		}

		ss << "\n";
	}

	return fmt::format("{}\n", ss.str());
}

//===============================================================

}
