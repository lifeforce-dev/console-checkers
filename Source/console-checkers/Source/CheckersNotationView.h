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
	CheckersNotationView(CheckersNotationView&& other) noexcept = default;
	CheckersNotationView& operator=(CheckersNotationView&& other) noexcept = default;

	CheckersNotationView();
	virtual ~CheckersNotationView() override;

	// Returns the textual representation of the board with checkers notation using colored numbers instead of glyphs.
	std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const override;

	// Given a string with a checkers coordinate the player understands, returns an index the game can understand.
	int32_t GameBoardPositionToIndex(const std::string& position) const override;

	// This view deals with single value positions on the board.
	std::string_view GetMoveCommandSyntax() const override;

	GameBoardViewStrategyId GetId() const override { return m_id; }

private:
	GameBoardViewStrategyId m_id = GameBoardViewStrategyId::CheckersNotation;
};

//===============================================================

}
