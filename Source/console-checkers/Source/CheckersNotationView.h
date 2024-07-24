//---------------------------------------------------------------
//
// CheckersNotationView.h
//

# pragma once

#include "IGameBoardViewStrategy.h"

namespace Checkers {

//===============================================================

class CheckersNotationView : public IGameBoardViewStrategy
{
public:

	// Returns the textual representation of the board with checkers notation using colored numbers instead of glyphs.
	[[nodiscard]]
	std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const override;

	// Given a string with a checkers coordinate the player understands, returns an index the game can understand.
	[[nodiscard]]
	int32_t GameBoardPositionToIndex(const std::string& position) const override;
};

//===============================================================

}
