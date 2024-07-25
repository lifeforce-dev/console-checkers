//---------------------------------------------------------------
//
// CheckersNotationView.h
//

# pragma once

#include "IGameBoardViewStrategy.h"

namespace Checkers {

//===============================================================

class CheckersNotationView final : public IGameBoardViewStrategy {
public:
	CheckersNotationView(const CheckersNotationView& other) = delete;
	CheckersNotationView& operator=(const CheckersNotationView& other) = delete;

	CheckersNotationView() = default;
	CheckersNotationView(CheckersNotationView&& other) noexcept = default;
	CheckersNotationView& operator=(CheckersNotationView&& other) noexcept = default;
	virtual ~CheckersNotationView() override;

	// Returns the textual representation of the board with checkers notation using colored numbers instead of glyphs.
	[[nodiscard]]
	std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const override;

	// Given a string with a checkers coordinate the player understands, returns an index the game can understand.
	[[nodiscard]]
	int32_t GameBoardPositionToIndex(const std::string& position) const override;
};

//===============================================================

}
